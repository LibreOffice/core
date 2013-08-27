#include <comphelper/configparser.hxx>
#include "rtl/ustring.hxx"

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>
#include <osl/file.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::xml::dom;
using namespace ::com::sun::star::uno;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::beans::PropertyValue;
using namespace ::comphelper;
namespace comphelper
{
    std::vector<rtl::OUString> ExpandedPathList;
    const std::vector<rtl::OUString>& Xml_ConfigParsing::InItExpandedFilePath(const rtl::OUString& filepath)
    {
        const Reference<css::uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        Reference< XMultiServiceFactory > xServiceFactory( xContext->getServiceManager() , UNO_QUERY_THROW );
        Reference< util::XMacroExpander > xMacroExpander = util::theMacroExpander::get( xContext );
        Reference< XMultiServiceFactory > xConfigProvider = configuration::theDefaultProvider::get( xContext );

        Any propValue = uno::makeAny(
            beans::PropertyValue(
                "nodepath", -1,
                uno::makeAny( OUString( "/org.openoffice.Office.Impress/Misc" )),
                beans::PropertyState_DIRECT_VALUE ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );

        Sequence< rtl::OUString > aFiles;
        xNameAccess->getByName( filepath ) >>= aFiles;

        rtl::OUString aFilename;
        for( sal_Int32 i=0; i < aFiles.getLength(); ++i )
        {
            aFilename = aFiles[i];
            if( aFilename.startsWith( "vnd.sun.star.expand:" ) )
            {
                // cut protocol
                rtl::OUString aMacro( aFilename.copy( sizeof ( "vnd.sun.star.expand:" ) -1 ) );

                // decode uric class chars
                aMacro = rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );

                // expand macro string
                aFilename = xMacroExpander->expandMacros( aMacro );
            }

            if( aFilename.startsWith( "file://" ) )
            {
                rtl::OUString aSysPath;
                if( osl_getSystemPathFromFileURL( aFilename.pData, &aSysPath.pData ) == osl_File_E_None )
                    aFilename = aSysPath;
            }
            ExpandedPathList.push_back(aFilename);
        }
        return ExpandedPathList;
    }
}