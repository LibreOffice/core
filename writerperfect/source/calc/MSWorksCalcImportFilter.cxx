/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksCalcImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/passwd.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>

#include <libwps/libwps.h>

#include <DocumentHandlerForOds.hxx>
#include <WPFTEncodingDialog.hxx>
#include <WPFTResMgr.hxx>
#include "MSWorksCalcImportFilter.hxx"
#include <strings.hrc>

#include <iostream>
#include <map>

using namespace ::com::sun::star;

namespace MSWorksCalcImportFilterInternal
{
/// returns the list of stream name present in a folder
static uno::Reference<sdbc::XResultSet>
getResultSet(const css::uno::Reference<css::ucb::XContent>& xPackageContent)
{
    try
    {
        if (xPackageContent.is())
        {
            ucbhelper::Content packageContent(xPackageContent,
                                              uno::Reference<ucb::XCommandEnvironment>(),
                                              comphelper::getProcessComponentContext());
            uno::Sequence<OUString> lPropNames{ "Title" };
            uno::Reference<sdbc::XResultSet> xResultSet(
                packageContent.createCursor(lPropNames, ucbhelper::INCLUDE_DOCUMENTS_ONLY));
            return xResultSet;
        }
        return uno::Reference<sdbc::XResultSet>();
    }
    catch (...)
    {
        SAL_WARN("writerperfect",
                 "ignoring Exception in MSWorksCalcImportFilterInternal:getResultSet");
        return uno::Reference<sdbc::XResultSet>();
    }
}

namespace
{
/** internal class used to create a structured RVNGInputStream from a list of path and their short names
 */
class FolderStream : public librevenge::RVNGInputStream
{
public:
    //! constructor
    explicit FolderStream(const css::uno::Reference<css::ucb::XContent>& xContent)
        : librevenge::RVNGInputStream()
        , m_xContent(xContent)
        , m_nameToPathMap()
    {
    }

    //! add a file
    void addFile(OUString const& path, std::string const& shortName)
    {
        m_nameToPathMap[shortName] = path;
    }
    /**! reads numbytes data.

     * \return a pointer to the read elements
     */
    const unsigned char* read(unsigned long, unsigned long&) override { return nullptr; }
    //! returns actual offset position
    long tell() override { return 0; }
    /*! \brief seeks to an offset position, from actual, beginning or ending position
     * \return 0 if ok
     */
    int seek(long, librevenge::RVNG_SEEK_TYPE) override { return 1; }
    //! returns true if we are at the end of the section/file
    bool isEnd() override { return true; }

    /** returns true if the stream is ole

     \sa returns always false*/
    bool isStructured() override { return true; }
    /** returns the number of sub streams.

     \sa returns always 2*/
    unsigned subStreamCount() override { return unsigned(m_nameToPathMap.size()); }
    /** returns the ith sub streams name */
    const char* subStreamName(unsigned id) override
    {
        if (m_nameToPathMap.size() < id)
            return nullptr;

        std::map<std::string, OUString>::const_iterator it = m_nameToPathMap.begin();
        std::advance(it, id);
        return it->first.c_str();
    }
    /** returns true if a substream with name exists */
    bool existsSubStream(const char* name) override
    {
        return name && m_nameToPathMap.find(name) != m_nameToPathMap.end();
    }
    /** return a new stream for an OLE zone */
    librevenge::RVNGInputStream* getSubStreamByName(const char* name) override
    {
        if (m_nameToPathMap.find(name) == m_nameToPathMap.end() || !m_xContent.is())
            return nullptr;

        try
        {
            const uno::Reference<sdbc::XResultSet> xResultSet = getResultSet(m_xContent);
            if (xResultSet.is() && xResultSet->first())
            {
                const uno::Reference<ucb::XContentAccess> xContentAccess(xResultSet,
                                                                         uno::UNO_QUERY_THROW);
                const uno::Reference<sdbc::XRow> xRow(xResultSet, uno::UNO_QUERY_THROW);
                OUString lPath = m_nameToPathMap.find(name)->second;
                do
                {
                    const OUString aTitle(xRow->getString(1));
                    if (aTitle != lPath)
                        continue;

                    const uno::Reference<ucb::XContent> xSubContent(xContentAccess->queryContent());
                    ucbhelper::Content aSubContent(xSubContent,
                                                   uno::Reference<ucb::XCommandEnvironment>(),
                                                   comphelper::getProcessComponentContext());
                    uno::Reference<io::XInputStream> xInputStream = aSubContent.openStream();
                    if (xInputStream.is())
                        return new writerperfect::WPXSvInputStream(xInputStream);
                    break;
                } while (xResultSet->next());
            }
        }
        catch (...)
        {
            SAL_WARN("writerperfect", "ignoring Exception in "
                                      "MSWorksCalcImportFilterInternal::FolderStream::"
                                      "getSubStreamByName");
        }

        return nullptr;
    }
    /** return a new stream for an OLE zone */
    librevenge::RVNGInputStream* getSubStreamById(unsigned id) override
    {
        char const* name = subStreamName(id);
        return name ? getSubStreamByName(name) : nullptr;
    }

private:
    /// the main container
    uno::Reference<ucb::XContent> m_xContent;
    /// the map short name to path
    std::map<std::string, OUString> m_nameToPathMap;
    FolderStream(const FolderStream&) = delete;
    FolderStream& operator=(const FolderStream&) = delete;
};
}
}

////////////////////////////////////////////////////////////
bool MSWorksCalcImportFilter::doImportDocument(weld::Window* pParent,
                                               librevenge::RVNGInputStream& rInput,
                                               OdsGenerator& rGenerator,
                                               utl::MediaDescriptor& mediaDescriptor)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding;
    const libwps::WPSConfidence confidence
        = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    if ((kind != libwps::WPS_SPREADSHEET && kind != libwps::WPS_DATABASE)
        || (confidence == libwps::WPS_CONFIDENCE_NONE))
        return false;
    std::string fileEncoding;
    if (needEncoding)
    {
        OUString encoding;
        // first check if we can find the encoding in the filter options (headless mode)
        mediaDescriptor[utl::MediaDescriptor::PROP_FILTEROPTIONS()] >>= encoding;
        if (!encoding.isEmpty()) // TODO: check if the encoding string is valid
            fileEncoding = encoding.toUtf8().getStr();
        else
        {
            OUString title;
            switch (creator)
            {
                case libwps::WPS_MSWORKS:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_MSWORKS);
                    encoding = "CP850";
                    break;
                case libwps::WPS_LOTUS:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_LOTUS);
                    encoding = "CP437";
                    break;
                case libwps::WPS_SYMPHONY:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_SYMPHONY);
                    encoding = "CP437";
                    break;
                case libwps::WPS_QUATTRO_PRO:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_QUATTROPRO);
                    encoding = "CP437";
                    break;
                case libwps::WPS_RESERVED_2:
                    title = WpResId(STR_ENCODING_DIALOG_TITLE_MSMULTIPLAN);
                    encoding = "CP437";
                    break;
                default:
                    SAL_INFO("writerperfect", "unexpected creator: " << creator);
                    title = WpResId(STR_ENCODING_DIALOG_TITLE);
                    encoding = "CP437";
                    break;
            }

            fileEncoding = encoding.toUtf8().getStr(); // set default to the proposed encoding
            try
            {
                writerperfect::WPFTEncodingDialog aDlg(pParent, title, encoding);
                if (aDlg.run() == RET_OK)
                {
                    if (!aDlg.GetEncoding().isEmpty())
                        fileEncoding = aDlg.GetEncoding().toUtf8().getStr();
                }
                // we can fail because we are in headless mode, the user has cancelled conversion, ...
                else if (aDlg.hasUserCalledCancel())
                    return false;
            }
            catch (...)
            {
                SAL_WARN("writerperfect",
                         "ignoring Exception in MSWorksCalcImportFilter::doImportDocument");
            }
        }
    }
    OString aUtf8Passwd;
    if (confidence == libwps::WPS_CONFIDENCE_SUPPORTED_ENCRYPTION)
    {
        OUString sPassword;
        // now check if we can find the password in the properties
        // (just in case, "soffice --headless" adds an option to send password)
        mediaDescriptor[utl::MediaDescriptor::PROP_PASSWORD()] >>= sPassword;
        if (!sPassword.isEmpty())
            aUtf8Passwd = OUStringToOString(sPassword, RTL_TEXTENCODING_UTF8);
        else
        {
            // ok, ask the user for a password
            try
            {
                SfxPasswordDialog aPasswdDlg(pParent);
                aPasswdDlg.SetMinLen(1);
                if (!aPasswdDlg.run())
                    return false;
                OUString aPasswd = aPasswdDlg.GetPassword();
                aUtf8Passwd = OUStringToOString(aPasswd, RTL_TEXTENCODING_UTF8);
            }
            catch (...)
            {
                return false;
            }
        }
    }
    return libwps::WPS_OK
           == libwps::WPSDocument::parse(&rInput, &rGenerator,
                                         confidence == libwps::WPS_CONFIDENCE_SUPPORTED_ENCRYPTION
                                             ? aUtf8Passwd.getStr()
                                             : nullptr,
                                         fileEncoding.c_str());
}

//XExtendedFilterDetection
sal_Bool
MSWorksCalcImportFilter::filter(const css::uno::Sequence<css::beans::PropertyValue>& rDescriptor)
{
    OUString sUrl;
    css::uno::Reference<css::io::XInputStream> xInputStream;
    css::uno::Reference<ucb::XContent> xContent;
    css::uno::Reference<css::awt::XWindow> xDialogParent;

    for (const auto& rValue : rDescriptor)
    {
        if (rValue.Name == "InputStream")
            rValue.Value >>= xInputStream;
        else if (rValue.Name == "UCBContent")
            rValue.Value >>= xContent;
        else if (rValue.Name == "FileName" || rValue.Name == "URL")
            rValue.Value >>= sUrl;
        else if (rValue.Name == "ParentWindow")
            rValue.Value >>= xDialogParent;
    }

    if (!getXContext().is() || !xInputStream.is())
    {
        OSL_ASSERT(false);
        return false;
    }

    // An XML import service: what we push sax messages to...
    css::uno::Reference<XInterface> xInternalFilter
        = getXContext()->getServiceManager()->createInstanceWithContext(
            writerperfect::DocumentHandlerFor<OdsGenerator>::name(), getXContext());
    assert(xInternalFilter);
    css::uno::Reference<css::xml::sax::XFastDocumentHandler> xInternalHandler(xInternalFilter,
                                                                              css::uno::UNO_QUERY);
    assert(xInternalHandler);

    // The XImporter sets up an empty target document for XDocumentHandler to write to...
    css::uno::Reference<css::document::XImporter> xImporter(xInternalHandler, css::uno::UNO_QUERY);
    assert(xImporter);
    xImporter->setTargetDocument(getTargetDocument());

    // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    writerperfect::DocumentHandler aHandler(
        new SvXMLLegacyToFastDocHandler(dynamic_cast<SvXMLImport*>(xInternalHandler.get())));

    writerperfect::WPXSvInputStream input(xInputStream);
    OdsGenerator exporter;
    exporter.addDocumentHandler(&aHandler, ODF_FLAT_XML);
    doRegisterHandlers(exporter);

    utl::MediaDescriptor aDescriptor(rDescriptor);
    try
    {
        // time to check if the file is a WK3 file and a FM3 file is
        // present
        bool checkForFM3 = false;
        if (input.seek(0, librevenge::RVNG_SEEK_SET) == 0 && xContent.is()
            && INetURLObject(sUrl).getExtension().equalsIgnoreAsciiCase("WK3"))
        {
            // check if the file header corresponds to a .wk3 file
            unsigned long numBytesRead;
            const unsigned char* data = input.read(6, numBytesRead);
            if (data && numBytesRead == 6 && data[0] == 0 && data[1] == 0 && data[2] == 0x1a
                && data[3] == 0 && data[4] < 2 && data[5] == 0x10)
                checkForFM3 = true;
        }
        if (checkForFM3)
        {
            // check if the format file exists
            const css::uno::Reference<container::XChild> xChild(xContent, uno::UNO_QUERY);
            if (xChild.is())
            {
                OUString sWM3Name;
                OUString sFM3Name;
                const css::uno::Reference<ucb::XContent> xPackageContent(xChild->getParent(),
                                                                         uno::UNO_QUERY);
                uno::Reference<sdbc::XResultSet> xResultSet
                    = MSWorksCalcImportFilterInternal::getResultSet(xPackageContent);
                if (xResultSet.is() && xResultSet->first())
                {
                    const uno::Reference<ucb::XContentAccess> xContentAccess(xResultSet,
                                                                             uno::UNO_QUERY_THROW);
                    const uno::Reference<sdbc::XRow> xRow(xResultSet, uno::UNO_QUERY_THROW);
                    INetURLObject aTmpUrl(sUrl);
                    sWM3Name = aTmpUrl.getName(INetURLObject::LAST_SEGMENT, true,
                                               INetURLObject::DecodeMechanism::WithCharset);
                    aTmpUrl.setExtension(u"FM3");
                    const OUString& sTestFM3Name
                        = aTmpUrl.getName(INetURLObject::LAST_SEGMENT, true,
                                          INetURLObject::DecodeMechanism::WithCharset);
                    do
                    {
                        const OUString& aTitle(xRow->getString(1));
                        if (aTitle.equalsIgnoreAsciiCase(sTestFM3Name))
                            sFM3Name = aTitle;
                    } while (xResultSet->next() && sFM3Name.isEmpty());
                }
                if (!sFM3Name.isEmpty())
                {
                    MSWorksCalcImportFilterInternal::FolderStream structuredInput(xPackageContent);
                    structuredInput.addFile(sWM3Name, "WK3");
                    structuredInput.addFile(sFM3Name, "FM3");

                    libwps::WPSKind kind = libwps::WPS_TEXT;
                    libwps::WPSCreator creator;
                    bool needEncoding;
                    const libwps::WPSConfidence confidence
                        = libwps::WPSDocument::isFileFormatSupported(&structuredInput, kind,
                                                                     creator, needEncoding);
                    if (confidence != libwps::WPS_CONFIDENCE_NONE)
                        return doImportDocument(Application::GetFrameWeld(xDialogParent),
                                                structuredInput, exporter, aDescriptor);
                }
            }
        }
    }
    catch (...)
    {
    }

    return doImportDocument(Application::GetFrameWeld(xDialogParent), input, exporter, aDescriptor);
}

bool MSWorksCalcImportFilter::doDetectFormat(librevenge::RVNGInputStream& rInput,
                                             OUString& rTypeName)
{
    libwps::WPSKind kind = libwps::WPS_TEXT;
    libwps::WPSCreator creator;
    bool needEncoding;
    const libwps::WPSConfidence confidence
        = libwps::WPSDocument::isFileFormatSupported(&rInput, kind, creator, needEncoding);

    if ((kind == libwps::WPS_SPREADSHEET || kind == libwps::WPS_DATABASE)
        && confidence != libwps::WPS_CONFIDENCE_NONE)
    {
        switch (creator)
        {
            case libwps::WPS_MSWORKS:
                rTypeName = "calc_MS_Works_Document";
                break;
            case libwps::WPS_LOTUS:
            case libwps::WPS_SYMPHONY:
                rTypeName = "calc_WPS_Lotus_Document";
                break;
            case libwps::WPS_QUATTRO_PRO:
                rTypeName = "calc_WPS_QPro_Document";
                break;
            case libwps::WPS_RESERVED_2:
                rTypeName = "calc_MS_Multiplan";
                break;
            default:
                break;
        }
    }

    return !rTypeName.isEmpty();
}

void MSWorksCalcImportFilter::doRegisterHandlers(OdsGenerator&) {}

// XServiceInfo
OUString SAL_CALL MSWorksCalcImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Calc.MSWorksCalcImportFilter";
}

sal_Bool SAL_CALL MSWorksCalcImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL MSWorksCalcImportFilter::getSupportedServiceNames()
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Calc_MSWorksCalcImportFilter_get_implementation(
    css::uno::XComponentContext* const context, const css::uno::Sequence<css::uno::Any>&)
{
    return cppu::acquire(new MSWorksCalcImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
