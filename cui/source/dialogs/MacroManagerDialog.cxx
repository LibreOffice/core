/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <MacroManagerDialog.hxx>

#include <bitmaps.hlst>
#include <cfgutil.hxx>
#include <dialmgr.hxx>
#include <scriptdlg.hxx>
#include <strings.hrc>
#include <basctl/basctldllpublic.hxx>
#include <basctl/sbxitem.hxx>
#include <basctl/scriptdocument.hxx>
#include <basic/basmgr.hxx>
#include <comphelper/SetFlagContextHelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/documentinfo.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/inputdlg.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svtools/imagemgr.hxx>
#include <svx/passwd.hxx>
#include <tools/urlobj.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/weldutils.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/theBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/XPersistentLibraryContainer.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

ScriptsListBox::ScriptsListBox(std::unique_ptr<weld::TreeView> xTreeView)
    : m_xTreeView(std::move(xTreeView))
    , m_xScratchIter(m_xTreeView->make_iterator())
{
    m_xTreeView->make_sorted();
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 35,
                                  m_xTreeView->get_height_rows(9));
    m_xTreeView->connect_query_tooltip(LINK(this, ScriptsListBox, QueryTooltip));
}

ScriptsListBox::~ScriptsListBox() { ClearAll(); }

void ScriptsListBox::ClearAll()
{
    sal_uInt16 nCount = aArr.size();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        ScriptInfo* pScriptInfo = aArr[i].get();
        if (pScriptInfo && pScriptInfo->pBrowseNode)
            pScriptInfo->pBrowseNode->release();
    }
    aArr.clear();
    m_xTreeView->clear();
}

void ScriptsListBox::Remove(const weld::TreeIter& rEntry)
{
    ScriptInfo* pScriptInfo = weld::fromId<ScriptInfo*>(m_xTreeView->get_id(rEntry));
    if (pScriptInfo)
    {
        if (pScriptInfo->pBrowseNode)
            pScriptInfo->pBrowseNode->release();
        for (auto it = aArr.begin(); it != aArr.end(); ++it)
        {
            if ((*it).get() == pScriptInfo)
            {
                aArr.erase(it);
                break;
            }
        }
    }
    m_xTreeView->remove(rEntry);
}

OUString ScriptsListBox::GetDescriptionText(const OUString& rId)
{
    ScriptInfo* pScriptInfo = weld::fromId<ScriptInfo*>(rId);
    if (pScriptInfo)
    {
        return pScriptInfo->sDescription;
    }
    return OUString();
}

OUString ScriptsListBox::GetSelectedScriptName()
{
    std::unique_ptr<weld::TreeIter> xScriptsEntryIter = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_selected(xScriptsEntryIter.get()))
        return OUString();
    return m_xTreeView->get_text(*xScriptsEntryIter);
}

IMPL_LINK(ScriptsListBox, QueryTooltip, const weld::TreeIter&, rEntryIter, OUString)
{
    return GetDescriptionText(m_xTreeView->get_id(rEntryIter));
}

ScriptContainersListBox::ScriptContainersListBox(std::unique_ptr<weld::TreeView> xTreeView,
                                                 MacroManagerDialog* pMacroManagerDialog)
    : m_pScriptsListBox(nullptr)
    , m_xTreeView(std::move(xTreeView))
    , m_pMacroManagerDialog(pMacroManagerDialog)
{
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 35,
                                  m_xTreeView->get_height_rows(9));
    m_xTreeView->connect_expanding(LINK(this, ScriptContainersListBox, ExpandingHdl));
    m_xTreeView->connect_query_tooltip(LINK(this, ScriptContainersListBox, QueryTooltip));
}

ScriptContainersListBox::~ScriptContainersListBox() { ClearAll(); }

void ScriptContainersListBox::ClearAll()
{
    sal_uInt16 nCount = m_xTreeView->n_children();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        ScriptContainerInfo* pScriptContainerInfo
            = weld::fromId<ScriptContainerInfo*>(m_xTreeView->get_id(i));
        if (pScriptContainerInfo)
        {
            if (pScriptContainerInfo->pBrowseNode)
                pScriptContainerInfo->pBrowseNode->release();
            delete pScriptContainerInfo;
        }
    }
    m_xTreeView->clear();
}

void ScriptContainersListBox::Init(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                                   const css::uno::Reference<css::frame::XFrame>& xFrame)
{
    m_xContext = xContext;
    m_xFrame = xFrame;
    m_xTreeView->freeze();
    Fill(nullptr);
    m_xTreeView->thaw();
    m_xTreeView->set_cursor(0);
}

void ScriptContainersListBox::Remove(const weld::TreeIter* pEntryIter, bool bRemoveEntryIter)
{
    // remove all children of pEntryIter
    std::unique_ptr<weld::TreeIter> xIter = m_xTreeView->make_iterator(pEntryIter);
    if (m_xTreeView->iter_has_child(*pEntryIter) && m_xTreeView->iter_children(*xIter))
    {
        // set xIter to the last child of pEntryIter
        do
        {
            std::unique_ptr<weld::TreeIter> xChildIter = m_xTreeView->make_iterator(xIter.get());
            while (m_xTreeView->iter_next_sibling(*xChildIter))
                m_xTreeView->copy_iterator(*xChildIter, *xIter);
        } while (m_xTreeView->iter_has_child(*xIter) && m_xTreeView->iter_children(*xIter));

        // children must be removed from the tree in reverse order
        std::unique_ptr<weld::TreeIter> xRemoveIter = m_xTreeView->make_iterator();
        while (m_xTreeView->iter_compare(*xIter, *pEntryIter) != 0)
        {
            m_xTreeView->copy_iterator(*xIter, *xRemoveIter);
            (void)m_xTreeView->iter_previous(*xIter);
            ScriptContainerInfo* pScriptContainerInfo
                = weld::fromId<ScriptContainerInfo*>(m_xTreeView->get_id(*xRemoveIter));
            if (pScriptContainerInfo)
            {
                if (pScriptContainerInfo->pBrowseNode)
                    pScriptContainerInfo->pBrowseNode->release();
                delete pScriptContainerInfo;
            }
            m_xTreeView->remove(*xRemoveIter);
        }
    }

    // maybe remove the entry
    if (bRemoveEntryIter)
    {
        ScriptContainerInfo* pScriptContainerInfo
            = weld::fromId<ScriptContainerInfo*>(m_xTreeView->get_id(*pEntryIter));
        if (pScriptContainerInfo)
        {
            if (pScriptContainerInfo->pBrowseNode)
                pScriptContainerInfo->pBrowseNode->release();
            delete pScriptContainerInfo;
        }
        m_xTreeView->remove(*pEntryIter);
    }
}

// inspired by CuiConfigGroupListBox::FillScriptList
// cui/source/customize/cfgutil.cxx
void ScriptContainersListBox::Fill(const weld::TreeIter* pEntryIter)
{
    weld::WaitObject aWait(m_pMacroManagerDialog->getDialog());

    css::uno::Reference<css::script::browse::XBrowseNode> xNode;
    if (pEntryIter == nullptr)
    {
        ClearAll();
        try
        {
            css::uno::Reference<css::script::browse::XBrowseNodeFactory> xFac
                = css::script::browse::theBrowseNodeFactory::get(
                    comphelper::getProcessComponentContext());
            xNode.set(
                xFac->createView(css::script::browse::BrowseNodeFactoryViewTypes::MACROORGANIZER));
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION(
                "cui.dialogs", "Caught some exception whilst retrieving browse nodes from factory");
            // TODO exception handling
            return;
        }
    }
    else
    {
        ScriptContainerInfo* pScriptContainerInfoEntry
            = weld::fromId<ScriptContainerInfo*>(m_xTreeView->get_id(*pEntryIter));

        xNode.set(pScriptContainerInfoEntry->pBrowseNode);

        Remove(pEntryIter, /*bRemoveEntryIter*/ false);
    }

    if (xNode->hasChildNodes())
    {
        // tdf#120362: Don't ask to enable disabled Java when filling script list
        css::uno::ContextLayer layer(comphelper::NoEnableJavaInteractionContext());

        bool bIsRootNode = false;

        OUString user(u"user"_ustr);
        OUString share(u"share"_ustr);
        OUString uno_packages(u"uno_packages"_ustr);
        if (xNode->getName() == "Root")
        {
            bIsRootNode = true;
        }

        //To mimic current starbasic behaviour we
        //need to make sure that only the current document
        //is displayed in the config tree. Tests below
        //set the bDisplay flag to FALSE if the current
        //node is a first level child of the Root and is NOT
        //either the current document, user or share
        //
        // NOTE: This approach doesn't work for open documents with the same name, for example
        // Untitled 1.odt and Untitled 1.ods will both be included as root containers.
        OUString currentDocTitle;
        if (css::uno::Reference<css::frame::XController> xController = m_xFrame->getController();
            xController.is())
        {
            css::uno::Reference<css::frame::XModel> xModel = xController->getModel();
            if (xModel.is())
                currentDocTitle = comphelper::DocumentInfo::getDocumentTitle(xModel);
        }

        const css::uno::Sequence<css::uno::Reference<css::script::browse::XBrowseNode>> children
            = xNode->getChildNodes();
        for (css::uno::Reference<css::script::browse::XBrowseNode> const& theChild : children)
        {
            if (!theChild.is())
                continue;

            if (theChild->getType() == css::script::browse::BrowseNodeTypes::SCRIPT)
                // we only want containers in this list box
                continue;
            if (theChild->getName() == uno_packages)
                continue;

            OUString uiName = theChild->getName();
            if (bIsRootNode)
            {
                if (uiName == user)
                {
                    uiName = CuiResId(RID_CUISTR_MYMACROS);
                }
                else if (uiName == share)
                {
                    uiName = CuiResId(RID_CUISTR_PRODMACROS);
                }
                else if (uiName != currentDocTitle)
                {
                    // as noted above
                    // NOTE: This approach doesn't work for open documents with the same name, for
                    // example, Untitled 1.odt and Untitled 1.ods will both be included as root
                    // containers.
                    continue;
                }
            }

            // We call acquire on the XBrowseNode so that it does not
            // get autodestructed and become invalid when accessed later.
            theChild->acquire();

            bool bChildOnDemand = false;

            if (theChild->hasChildNodes())
            {
                const css::uno::Sequence<css::uno::Reference<css::script::browse::XBrowseNode>>
                    grandchildren = theChild->getChildNodes();
                for (const auto& rxNode : grandchildren)
                {
                    if (!rxNode.is())
                        continue;
                    if (rxNode->getType() == css::script::browse::BrowseNodeTypes::CONTAINER)
                    {
                        bChildOnDemand = true;
                        break;
                    }
                }
            }

            OUString aImage = CuiConfigGroupListBox::GetImage(theChild, m_xContext, bIsRootNode);
            Insert(theChild, pEntryIter, uiName, aImage, bChildOnDemand);
        }
    }

    // add Basic dialogs
    if (pEntryIter && m_xTreeView->get_iter_depth(*pEntryIter) == 2) // library
    {
        // parent node of entry node is a language node
        std::unique_ptr<weld::TreeIter> xIter = m_xTreeView->make_iterator(pEntryIter);
        m_xTreeView->iter_parent(*xIter); // language
        if (m_xTreeView->get_text(*xIter) == "Basic")
        {
            basctl::ScriptDocument aDocument = GetScriptDocument(pEntryIter);
            if (!aDocument.isAlive())
            {
                return;
            }

            OUString aLibName = m_xTreeView->get_text(*pEntryIter);

            css::uno::Reference<css::script::XLibraryContainer> xDlgLibContainer(
                aDocument.getLibraryContainer(basctl::E_DIALOGS));

            if (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
                && !xDlgLibContainer->isLibraryLoaded(aLibName))
            {
                xDlgLibContainer->loadLibrary(aLibName);
            }

            if (!(xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
                  && xDlgLibContainer->isLibraryLoaded(aLibName)))
            {
                return;
            }

            for (const OUString& rDlgName : aDocument.getObjectNames(basctl::E_DIALOGS, aLibName))
            {
                Insert(nullptr, pEntryIter, rDlgName, RID_CUIBMP_DIALOG, false);
            }
        }
    }
}

void ScriptContainersListBox::Insert(
    const css::uno::Reference<css::script::browse::XBrowseNode>& xInsertNode,
    const weld::TreeIter* pIter, const OUString& rsUiName, const OUString& rsImage,
    bool bChildOnDemand, int nPos, weld::TreeIter* pRet)
{
    std::unique_ptr<weld::TreeIter> xNewEntryIter = m_xTreeView->make_iterator();

    OUString sId(weld::toId(new ScriptContainerInfo(xInsertNode.get())));
    m_xTreeView->insert(pIter, nPos, &rsUiName, &sId, nullptr, nullptr, bChildOnDemand,
                        xNewEntryIter.get());
    m_xTreeView->set_image(*xNewEntryIter, rsImage);

    //  set password and linked image only for Basic libraries
    if (m_xTreeView->get_iter_depth(*xNewEntryIter) == 2) // library
    {
        std::unique_ptr<weld::TreeIter> xLanguageIter
            = m_xTreeView->make_iterator(xNewEntryIter.get());
        m_xTreeView->iter_parent(*xLanguageIter);
        if (m_xTreeView->get_text(*xLanguageIter) == "Basic")
        {
            basctl::ScriptDocument aDocument = GetScriptDocument(xNewEntryIter.get());
            if (!aDocument.isAlive())
                return;

            OUString aLibName = m_xTreeView->get_text(*xNewEntryIter);

            css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                aDocument.getLibraryContainer(basctl::E_SCRIPTS));
            if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName))
            {
                css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(
                    xModLibContainer, css::uno::UNO_QUERY);
                if (xPasswd.is() && xPasswd->isLibraryPasswordProtected(aLibName))
                {
                    // password protected
                    m_xTreeView->set_image(*xNewEntryIter, RID_CUIBMP_LOCKED);
                }
                else if (xModLibContainer->isLibraryLink(aLibName))
                {
                    // linked
                    m_xTreeView->set_image(*xNewEntryIter, RID_CUIBMP_LINKED);
                }
            }
        }
    }
    if (pRet)
        pRet = xNewEntryIter.get();
}

// fills the scripts list box
// inspired by code in void CommandCategoryListBox::addChildren
// cui/source/customize/CommandCategoryListBox.cxx
// and void CuiConfigGroupListBox::GroupSelected()
// cui/source/customize/cfgutil.cxx
void ScriptContainersListBox::ScriptContainerSelected()
{
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xIter.get()))
        return;

    m_pScriptsListBox->freeze();
    m_pScriptsListBox->ClearAll();

    if (!m_xTreeView->iter_has_child(*xIter) && !m_xTreeView->get_children_on_demand(*xIter))
    {
        // maybe the browse node has children and those children are most likely script nodes
        ScriptContainerInfo* pScriptContainerInfo
            = weld::fromId<ScriptContainerInfo*>(m_xTreeView->get_id(*xIter));
        css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode(
            pScriptContainerInfo->pBrowseNode);
        try
        {
            if (xBrowseNode->hasChildNodes())
            {
                const css::uno::Sequence<css::uno::Reference<css::script::browse::XBrowseNode>>
                    children = xBrowseNode->getChildNodes();

                for (const css::uno::Reference<css::script::browse::XBrowseNode>& childNode :
                     children)
                {
                    if (!childNode.is())
                        continue;

                    if (childNode->getType() == css::script::browse::BrowseNodeTypes::SCRIPT)
                    {
                        css::uno::Reference<css::beans::XPropertySet> xPropSet(childNode,
                                                                               css::uno::UNO_QUERY);
                        if (!xPropSet.is())
                        {
                            continue;
                        }

                        OUString sURI;
                        try
                        {
                            css::uno::Any value = xPropSet->getPropertyValue(u"URI"_ustr);
                            value >>= sURI;
                        }
                        catch (css::uno::Exception&)
                        {
                            // do nothing, the URI will be empty
                        }

                        OUString sDescription;
                        try
                        {
                            css::uno::Any value = xPropSet->getPropertyValue(u"Description"_ustr);
                            value >>= sDescription;
                        }
                        catch (css::uno::Exception&)
                        {
                            // do nothing, the description will be empty
                        }

                        childNode->acquire();

                        m_pScriptsListBox->aArr.push_back(
                            std::make_unique<ScriptInfo>(childNode.get(), sURI, sDescription));

                        OUString sId(weld::toId(m_pScriptsListBox->aArr.back().get()));
                        m_pScriptsListBox->append(sId, childNode->getName(), RID_CUIBMP_MACRO);
                    }
                }
            }
        }
        catch (css::uno::RuntimeException&)
        {
            // do nothing, the entry will not be displayed in the UI
        }
    }

    m_pScriptsListBox->thaw();

    if (m_pScriptsListBox->n_children())
        m_pScriptsListBox->select(0);
}

OUString ScriptContainersListBox::GetContainerName(const weld::TreeIter& rIter,
                                                   const ScriptContainerType eScriptContainerType)
{
    weld::TreeView& rScriptContainersTreeView = *m_xTreeView;
    std::unique_ptr<weld::TreeIter> xSelectedEntryIter
        = rScriptContainersTreeView.make_iterator(&rIter);

    std::unique_ptr<weld::TreeIter> xIter
        = rScriptContainersTreeView.make_iterator(xSelectedEntryIter.get());

    int nEntryDepth;
    while ((nEntryDepth = rScriptContainersTreeView.get_iter_depth(*xIter)))
    {
        if ((nEntryDepth == 3 && eScriptContainerType == ScriptContainerType::MODULEORDIALOG)
            || (nEntryDepth == 2 && eScriptContainerType == ScriptContainerType::LIBRARY)
            || (nEntryDepth == 1 && eScriptContainerType == ScriptContainerType::LANGUAGE))
            return rScriptContainersTreeView.get_text(*xIter);
        rScriptContainersTreeView.iter_parent(*xIter);
    }
    if (eScriptContainerType == ScriptContainerType::LOCATION)
        return rScriptContainersTreeView.get_text(*xIter);

    return OUString();
}

OUString
ScriptContainersListBox::GetSelectedEntryContainerName(ScriptContainerType eScriptContainerType)
{
    std::unique_ptr<weld::TreeIter> xSelectedEntryIter = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_selected(xSelectedEntryIter.get()))
        return OUString(); // should never happen
    return GetContainerName(*xSelectedEntryIter, eScriptContainerType);
}

IMPL_LINK(ScriptContainersListBox, QueryTooltip, const weld::TreeIter&, rEntryIter, OUString)
{
    // for Basic library check for linked library
    if (m_xTreeView->get_iter_depth(rEntryIter) == 2) // library
    {
        // language
        std::unique_ptr<weld::TreeIter> xLanguageIter = m_xTreeView->make_iterator(&rEntryIter);
        m_xTreeView->iter_parent(*xLanguageIter);
        if (m_xTreeView->get_text(*xLanguageIter) == "Basic")
        {
            basctl::ScriptDocument aDocument = GetScriptDocument(&rEntryIter);
            if (!aDocument.isAlive())
                return OUString();

            css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                aDocument.getLibraryContainer(basctl::E_SCRIPTS));
            // check for linked library
            OUString aLibName = m_xTreeView->get_text(rEntryIter);
            if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
                && xModLibContainer->isLibraryLink(aLibName))
            {
                OUString aLinkURL = xModLibContainer->getLibraryLinkURL(aLibName);
                osl::File::getSystemPathFromFileURL(aLinkURL, aLinkURL);
                return aLinkURL;
            }
        }
    }
    return OUString();
}

IMPL_LINK(ScriptContainersListBox, ExpandingHdl, const weld::TreeIter&, rEntryIter, bool)
{
    // for Basic library check for password protected
    if (m_xTreeView->get_iter_depth(rEntryIter) == 2) // library
    {
        // language
        std::unique_ptr<weld::TreeIter> xLanguageIter = m_xTreeView->make_iterator(&rEntryIter);
        m_xTreeView->iter_parent(*xLanguageIter);
        if (m_xTreeView->get_text(*xLanguageIter) == "Basic")
        {
            basctl::ScriptDocument aDocument = GetScriptDocument(&rEntryIter);
            if (!aDocument.isAlive())
                return false; // not expanded

            OUString aLibName = m_xTreeView->get_text(rEntryIter);

            // check if the library is password protected
            css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                aDocument.getLibraryContainer(basctl::E_SCRIPTS));
            if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName))
            {
                css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(
                    xModLibContainer, css::uno::UNO_QUERY);
                if (xPasswd.is() && xPasswd->isLibraryPasswordProtected(aLibName)
                    && !xPasswd->isLibraryPasswordVerified(aLibName))
                {
                    // ensure selection before password dialog is shown
                    m_xTreeView->select(rEntryIter);
                    OUString sPassword;
                    if (!basctl::QueryPassword(m_pMacroManagerDialog->getDialog(), xModLibContainer,
                                               aLibName, sPassword, true, true))
                    {
                        return false; // indicates the expansion of the row is refused
                    }
                }
            }
        }
    }

    if (m_xTreeView->get_children_on_demand(rEntryIter))
    {
        Fill(&rEntryIter);
    }

    return true;
}

MacroManagerDialog::MacroManagerDialog(weld::Window* pParent,
                                       const css::uno::Reference<css::frame::XFrame>& xDocFrame)
    : GenericDialogController(pParent, u"cui/ui/macromanagerdialog.ui"_ustr,
                              u"MacroManagerDialog"_ustr)
    , m_xDocumentFrame(xDocFrame)
    , m_xScriptContainersListBox(
          new ScriptContainersListBox(m_xBuilder->weld_tree_view(u"scriptcontainers"_ustr), this))
    , m_xScriptsListBox(new ScriptsListBox(m_xBuilder->weld_tree_view(u"scripts"_ustr)))
    , m_xScriptContainersListBoxLabel(m_xBuilder->weld_label(u"scriptcontainerlistboxlabel"_ustr))
    , m_xScriptsListBoxLabel(m_xBuilder->weld_label(u"scriptslistboxlabel"_ustr))
    , m_xRunButton(m_xBuilder->weld_button(u"run"_ustr))
    , m_xCloseButton(m_xBuilder->weld_button(u"close"_ustr))
    , m_xDescriptionText(m_xBuilder->weld_text_view(u"description"_ustr))
    , m_xDescriptionFrame(m_xBuilder->weld_frame(u"descriptionframe"_ustr))
    , m_xNewLibraryButton(m_xBuilder->weld_button(u"newlibrary"_ustr))
    , m_xNewModuleButton(m_xBuilder->weld_button(u"newmodule"_ustr))
    , m_xNewDialogButton(m_xBuilder->weld_button(u"newdialog"_ustr))
    , m_xLibraryModuleDialogEditButton(m_xBuilder->weld_button(u"librarymoduledialogedit"_ustr))
    , m_xLibraryModuleDialogRenameButton(m_xBuilder->weld_button(u"librarymoduledialogrename"_ustr))
    , m_xLibraryModuleDialogDeleteButton(m_xBuilder->weld_button(u"librarymoduledialogdelete"_ustr))
    , m_xLibraryPasswordButton(m_xBuilder->weld_button(u"librarypassword"_ustr))
    , m_xLibraryImportButton(m_xBuilder->weld_button(u"libraryimport"_ustr))
    , m_xLibraryExportButton(m_xBuilder->weld_button(u"libraryexport"_ustr))
    , m_xMacroEditButton(m_xBuilder->weld_button(u"macroedit"_ustr))
    , m_xMacroDeleteButton(m_xBuilder->weld_button(u"macrodelete"_ustr))
    , m_xMacroCreateButton(m_xBuilder->weld_button(u"macrocreate"_ustr))
    , m_xMacroRenameButton(m_xBuilder->weld_button(u"macrorename"_ustr))
    , m_xAssignButton(m_xBuilder->weld_button(u"assign"_ustr))
{
    m_aScriptsListBoxLabelBaseStr = m_xScriptsListBoxLabel->get_label();

    m_xScriptContainersListBox->SetScriptsListBox(m_xScriptsListBox.get());
    m_xScriptContainersListBox->Init(comphelper::getProcessComponentContext(), m_xDocumentFrame);

    m_xScriptContainersListBox->connect_changed(LINK(this, MacroManagerDialog, SelectHdl));

    m_xScriptsListBox->connect_changed(LINK(this, MacroManagerDialog, SelectHdl));
    m_xScriptsListBox->connect_row_activated(
        LINK(this, MacroManagerDialog, FunctionDoubleClickHdl));
    m_xScriptsListBox->connect_popup_menu(LINK(this, MacroManagerDialog, ContextMenuHdl));

    m_xAssignButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xRunButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xCloseButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xNewLibraryButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xNewModuleButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xNewDialogButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xLibraryModuleDialogEditButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xLibraryModuleDialogRenameButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xLibraryModuleDialogDeleteButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xLibraryPasswordButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xLibraryImportButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xLibraryExportButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xMacroCreateButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xMacroEditButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xMacroRenameButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));
    m_xMacroDeleteButton->connect_clicked(LINK(this, MacroManagerDialog, ClickHdl));

    StartListening(*SfxGetpApp());
}

MacroManagerDialog::~MacroManagerDialog() { EndListening(*SfxGetpApp()); }

void MacroManagerDialog::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::ScriptDocumentChanged)
    {
        weld::TreeView& rScriptContainersTreeView = m_xScriptContainersListBox->get_widget();

        auto get_path = [&rScriptContainersTreeView](const weld::TreeIter* pIter) {
            std::unique_ptr<weld::TreeIter> xIter = rScriptContainersTreeView.make_iterator(pIter);
            OUString sPath = rScriptContainersTreeView.get_text(*xIter);
            while (rScriptContainersTreeView.iter_parent(*xIter))
                sPath = rScriptContainersTreeView.get_text(*xIter) + "|" + sPath;
            return sPath;
        };

        std::unique_ptr<weld::TreeIter> xIter = rScriptContainersTreeView.make_iterator();

        // for use to restore the script container tree scroll position
        int nOldScrollPos = rScriptContainersTreeView.vadjustment_get_value();

        // save the current path of the selection in the script containers list for reselection
        // and the selected entry in the scripts list box
        OUString sScriptContainersListBoxSelectedEntryPath;
        OUString sScriptsListBoxSelectedEntry;
        if (rScriptContainersTreeView.get_selected(xIter.get()))
        {
            sScriptContainersListBoxSelectedEntryPath = get_path(xIter.get());
            sScriptsListBoxSelectedEntry = m_xScriptsListBox->GetSelectedScriptName();
        }

        // create a set containing paths for use to restore the script containers tree expand state
        std::unordered_set<OUString> aExpandedSet;
        if (!rScriptContainersTreeView.get_iter_first(*xIter)) // no entries?
            return;
        do
        {
            if (rScriptContainersTreeView.get_row_expanded(*xIter))
                aExpandedSet.insert(get_path(xIter.get()));
        } while (rScriptContainersTreeView.iter_next(*xIter));

        // fill
        rScriptContainersTreeView.freeze();
        m_xScriptContainersListBox->Fill(nullptr);
        rScriptContainersTreeView.thaw();

        // restore script container tree expand state or at least try
        if (!rScriptContainersTreeView.get_iter_first(*xIter))
        {
            UpdateUI();
            return;
        }
        do
        {
            if (aExpandedSet.erase(get_path(xIter.get())))
                rScriptContainersTreeView.expand_row(*xIter);
        } while (aExpandedSet.size() && rScriptContainersTreeView.iter_next(*xIter));

        // restore the script containers tree scroll position
        rScriptContainersTreeView.vadjustment_set_value(nOldScrollPos);

        // if possible select the saved scripts container list entry
        if (sScriptContainersListBoxSelectedEntryPath.isEmpty())
        {
            UpdateUI();
            return;
        }
        bool bFound = false;
        while (true)
        {
            if (!rScriptContainersTreeView.get_iter_first(*xIter))
            {
                UpdateUI();
                return;
            }
            do
            {
                if (get_path(xIter.get()) == sScriptContainersListBoxSelectedEntryPath)
                {
                    rScriptContainersTreeView.select(*xIter);
                    bFound = true;
                    break;
                }
            } while (rScriptContainersTreeView.iter_next(*xIter));
            if (bFound)
                break;
            // when not found remove the last node from path and try again
            auto i = sScriptContainersListBoxSelectedEntryPath.lastIndexOf('|');
            if (i == -1)
            {
                // didn't find
                UpdateUI();
                return;
            }
            sScriptContainersListBoxSelectedEntryPath
                = sScriptContainersListBoxSelectedEntryPath.copy(0, i);
        }

        // fill the scripts list box and set label
        m_xScriptContainersListBox->ScriptContainerSelected();
        m_xScriptsListBoxLabel->set_label(m_aScriptsListBoxLabelBaseStr + " "
                                          + rScriptContainersTreeView.get_text(*xIter));

        // reselect the macro in the scripts list box
        if (!sScriptsListBoxSelectedEntry.isEmpty())
        {
            weld::TreeView& rScriptsTreeView = m_xScriptsListBox->get_widget();
            xIter = rScriptsTreeView.make_iterator();
            if (!rScriptsTreeView.get_iter_first(*xIter))
            {
                UpdateUI();
                return;
            }
            bool bIsIterValid = true;
            while (bIsIterValid
                   && rScriptsTreeView.get_text(*xIter) != sScriptsListBoxSelectedEntry)
                bIsIterValid = rScriptsTreeView.iter_next_sibling(*xIter);
            if (bIsIterValid)
            {
                rScriptsTreeView.scroll_to_row(*xIter);
                rScriptsTreeView.select(*xIter);
            }
        }

        UpdateUI();
    }
}

IMPL_LINK(MacroManagerDialog, SelectHdl, weld::TreeView&, rTreeView, void)
{
    if (&rTreeView == &m_xScriptContainersListBox->get_widget())
    {
        m_xScriptsListBoxLabel->set_label(m_aScriptsListBoxLabelBaseStr);
        m_xScriptsListBox->ClearAll();

        std::unique_ptr<weld::TreeIter> xSelectedEntryIter = rTreeView.make_iterator();
        if (!rTreeView.get_selected(xSelectedEntryIter.get()))
        {
            UpdateUI();
            return;
        }

        auto nSelectedEntryDepth = rTreeView.get_iter_depth(*xSelectedEntryIter);
        bool bBasic = m_xScriptContainersListBox->GetContainerName(*xSelectedEntryIter,
                                                                   ScriptContainerType::LANGUAGE)
                      == u"Basic"_ustr;

        if (nSelectedEntryDepth > 1)
        {
            if (nSelectedEntryDepth > (!bBasic ? 1 : 2))
            {
                ScriptContainerInfo* pScriptContainerInfo
                    = weld::fromId<ScriptContainerInfo*>(rTreeView.get_id(*xSelectedEntryIter));
                // pBrowseNode is nullptr for Basic Dialog entries
                if (pScriptContainerInfo->pBrowseNode)
                {
                    // fill the scripts list box and set the label
                    m_xScriptContainersListBox->ScriptContainerSelected();
                    m_xScriptsListBoxLabel->set_label(m_aScriptsListBoxLabelBaseStr + " "
                                                      + rTreeView.get_text(*xSelectedEntryIter));
                }
            }
        }
    }
    UpdateUI();
}

// IMPL_LINK_NOARG(SvxScriptSelectorDialog, FunctionDoubleClickHdl, weld::TreeView&, bool)
// cui/source/customize/cfgutil.cxx
IMPL_LINK_NOARG(MacroManagerDialog, FunctionDoubleClickHdl, weld::TreeView&, bool)
{
    if (m_xRunButton->get_sensitive())
        ClickHdl(*m_xRunButton);
    return true;
}

// IMPL_LINK(SvxScriptSelectorDialog, ContextMenuHdl, const CommandEvent&, rCEvt, bool)
// cui/source/customize/cfgutil.cxx
// and
// IMPL_LINK(MacroChooser, ContextMenuHdl, const CommandEvent&, rCEvt, bool)
// basctl/source/basicide/macrodlg.cxx
IMPL_LINK(MacroManagerDialog, ContextMenuHdl, const CommandEvent&, rCEvt, bool)
{
    weld::TreeView& xTreeView = m_xScriptsListBox->get_widget();
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu || !xTreeView.n_children())
        return false;

    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(&xTreeView, u"modules/BasicIDE/ui/sortmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu(u"sortmenu"_ustr));
    std::unique_ptr<weld::Menu> xDropMenu(xBuilder->weld_menu(u"sortsubmenu"_ustr));
    xDropMenu->set_active(u"alphabetically"_ustr, xTreeView.get_sort_order());
    xDropMenu->set_active(u"properorder"_ustr, !xTreeView.get_sort_order());

    OUString sCommand(
        xPopup->popup_at_rect(&xTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1))));
    if (sCommand == "alphabetically")
    {
        xTreeView.make_sorted();
    }
    else if (sCommand == "properorder")
    {
        xTreeView.make_unsorted();
        m_xScriptContainersListBox->ScriptContainerSelected();
    }
    else if (!sCommand.isEmpty())
    {
        SAL_WARN("cui.dialogs", "Unknown context menu action: " << sCommand);
    }

    return true;
}

// same as OUString SvxScriptOrgDialog::getBoolProperty((Reference<beans::XPropertySet> const& xProps, OUString const& propName)
// cui/source/dialogs/scriptdlg.cxx
bool MacroManagerDialog::getBoolProperty(
    css::uno::Reference<css::beans::XPropertySet> const& xProps, OUString const& propName)
{
    bool result = false;
    try
    {
        xProps->getPropertyValue(propName) >>= result;
    }
    catch (css::uno::Exception&)
    {
        return result;
    }
    return result;
}

css::uno::Reference<css::script::browse::XBrowseNode>
MacroManagerDialog::getBrowseNode(const weld::TreeView& rTreeView, const weld::TreeIter& rTreeIter)
{
    if (&rTreeView == m_xScriptContainersListBox->m_xTreeView.get())
    {
        ScriptContainerInfo* pScriptContainerInfo;
        pScriptContainerInfo = weld::fromId<ScriptContainerInfo*>(rTreeView.get_id(rTreeIter));
        if (pScriptContainerInfo)
            return pScriptContainerInfo->pBrowseNode;
    }
    else
    {
        ScriptInfo* pScriptInfo = weld::fromId<ScriptInfo*>(rTreeView.get_id(rTreeIter));
        if (pScriptInfo)
            return pScriptInfo->pBrowseNode;
    }
    return nullptr;
}

void MacroManagerDialog::UpdateUI()
{
    OUString sDescriptionText
        = ScriptsListBox::GetDescriptionText(m_xScriptsListBox->get_selected_id());

    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (rTreeView.get_selected(xSelectedIter.get())
        && rTreeView.get_iter_depth(*xSelectedIter) == 2) // library
    {
        if (m_xScriptContainersListBox->GetSelectedEntryContainerName(ScriptContainerType::LANGUAGE)
            == "Basic")
        {
            basctl::ScriptDocument aDocument
                = m_xScriptContainersListBox->GetScriptDocument(xSelectedIter.get());
            if (aDocument.isAlive())
            {
                // if this is a Basic linked library use the link url name for the description string
                css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                    aDocument.getLibraryContainer(basctl::E_SCRIPTS));
                OUString aLibName = rTreeView.get_text(*xSelectedIter);
                if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
                    && xModLibContainer->isLibraryLink(aLibName))
                {
                    OUString aLinkURL = xModLibContainer->getLibraryLinkURL(aLibName);
                    OUString aSysPath;
                    if (osl::File::getSystemPathFromFileURL(aLinkURL, aSysPath)
                        == osl::FileBase::E_None)
                        sDescriptionText = aSysPath;
                    else
                        sDescriptionText = aLinkURL;
                }
            }
        }
    }

    m_xDescriptionText->set_text(sDescriptionText);
    CheckButtons();
}

void MacroManagerDialog::CheckButtons()
{
    bool bSensitiveNewLibraryButton = false;
    bool bSensitiveNewModuleButton = false;
    bool bSensitiveNewDialogButton = false;
    bool bSensitiveLibraryModuleDialogEditButton = false;
    bool bSensitiveLibraryModuleDialogRenameButton = false;
    bool bSensitiveLibraryModuleDialogDeleteButton = false;
    bool bSensitiveLibraryPasswordButton = false;
    bool bSensitiveLibraryImportButton = false;
    bool bSensitiveLibraryExportButton = false;

    bool bSensitiveMacroRunButton = false;
    bool bSensitiveMacroCreateButton = false;
    bool bSensitiveMacroEditButton = false;
    bool bSensitiveMacroRenameButton = false;
    bool bSensitiveMacroDeleteButton = false;

    bool bSensitiveAssignButton = false;

    weld::TreeView& rScriptContainersTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xScriptContainersSelectedIter
        = rScriptContainersTreeView.make_iterator();
    if (rScriptContainersTreeView.get_selected(xScriptContainersSelectedIter.get()))
    {
        if (auto nSelectedIterDepth
            = rScriptContainersTreeView.get_iter_depth(*xScriptContainersSelectedIter))
        {
            std::unique_ptr<weld::TreeIter> xLocationContainerIter
                = rScriptContainersTreeView.make_iterator(xScriptContainersSelectedIter.get());
            while (rScriptContainersTreeView.get_iter_depth(*xLocationContainerIter))
                rScriptContainersTreeView.iter_parent(*xLocationContainerIter);
            bool bSharedLocationContainer
                = getBrowseNode(rScriptContainersTreeView, *xLocationContainerIter)->getName()
                  == u"share"_ustr;

            bool bBasic = m_xScriptContainersListBox->GetSelectedEntryContainerName(
                              ScriptContainerType::LANGUAGE)
                          == u"Basic"_ustr;

            if (nSelectedIterDepth == 1) // language node
            {
                if (!bSharedLocationContainer)
                {
                    if (bBasic)
                    {
                        bSensitiveNewLibraryButton = true;
                        bSensitiveLibraryImportButton = true;
                    }
                    else
                    {
                        css::uno::Reference<css::script::browse::XBrowseNode> node = getBrowseNode(
                            rScriptContainersTreeView, *xScriptContainersSelectedIter);
                        if (node.is())
                        {
                            css::uno::Reference<css::beans::XPropertySet> xProps(
                                node, css::uno::UNO_QUERY);
                            if (xProps.is())
                            {
                                if (getBoolProperty(xProps, "Creatable"))
                                {
                                    bSensitiveNewLibraryButton = true;
                                }
                            }
                        }
                    }
                }
            }
            else if (bBasic && nSelectedIterDepth == 2) // library node
            {
                basctl::ScriptDocument aDocument = m_xScriptContainersListBox->GetScriptDocument();
                if (!aDocument.isAlive())
                    return;

                OUString aLibName = m_xScriptContainersListBox->GetSelectedEntryContainerName(
                    ScriptContainerType::LIBRARY);

                css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(
                    aDocument.getLibraryContainer(basctl::E_SCRIPTS), css::uno::UNO_QUERY);

                if (xPasswd.is() && xPasswd->isLibraryPasswordProtected(aLibName)
                    && !xPasswd->isLibraryPasswordVerified(aLibName))
                {
                    bSensitiveLibraryPasswordButton = true;
                }
                else if (bSharedLocationContainer)
                {
                    bSensitiveLibraryModuleDialogEditButton = true;
                }
                else
                {
                    // check, if library is readonly
                    css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                        aDocument.getLibraryContainer(basctl::E_SCRIPTS));

                    bool bReadOnly = xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
                                     && xModLibContainer->isLibraryReadOnly(aLibName);

                    bSensitiveNewModuleButton = !bReadOnly;
                    bSensitiveNewDialogButton = !bReadOnly;
                    bSensitiveLibraryModuleDialogEditButton = true;
                    if (aLibName != u"Standard"_ustr) // need to use a ResId?
                    {
                        bSensitiveLibraryModuleDialogRenameButton = !bReadOnly;
                        bSensitiveLibraryModuleDialogDeleteButton = true; //!bReadOnly;
                        bSensitiveLibraryPasswordButton = !bReadOnly;
                        // why not export for "Standard"?
                        bSensitiveLibraryExportButton = true;
                    }
                }
            }
            else if (bBasic && nSelectedIterDepth == 3) // module/dialog node
            {
                if (bSharedLocationContainer)
                {
                    bSensitiveLibraryModuleDialogEditButton = true;
                }
                else
                {
                    basctl::ScriptDocument aDocument
                        = m_xScriptContainersListBox->GetScriptDocument();
                    assert(aDocument.isAlive());

                    OUString aLibName = m_xScriptContainersListBox->GetSelectedEntryContainerName(
                        ScriptContainerType::LIBRARY);

                    css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                        aDocument.getLibraryContainer(basctl::E_SCRIPTS));
                    css::uno::Reference<css::script::XLibraryContainer2> xDlgLibContainer(
                        aDocument.getLibraryContainer(basctl::E_DIALOGS));

                    bool bReadOnly
                        = (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
                           && (xModLibContainer->isLibraryReadOnly(aLibName)
                               || xModLibContainer->isLibraryLink(aLibName)))
                          || (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
                              && (xDlgLibContainer->isLibraryReadOnly(aLibName)
                                  || xDlgLibContainer->isLibraryLink(aLibName)));

                    bSensitiveLibraryModuleDialogEditButton = true;
                    bSensitiveLibraryModuleDialogRenameButton = !bReadOnly;
                    bSensitiveLibraryModuleDialogDeleteButton = !bReadOnly;
                }
            }

            if (!bSharedLocationContainer && nSelectedIterDepth > 1)
            {
                css::uno::Reference<css::script::browse::XBrowseNode> node
                    = getBrowseNode(rScriptContainersTreeView, *xScriptContainersSelectedIter);
                if (node.is())
                {
                    css::uno::Reference<css::beans::XPropertySet> xProps(node, css::uno::UNO_QUERY);
                    if (xProps.is())
                    {
                        if (getBoolProperty(xProps, "Creatable")
                            && rScriptContainersTreeView.get_iter_depth(
                                   *xScriptContainersSelectedIter)
                                   == 2) // library entry
                        {
                            bSensitiveMacroCreateButton = true;
                        }
                        if (getBoolProperty(xProps, "Editable"))
                        {
                            bSensitiveLibraryModuleDialogEditButton = true;
                        }
                        if (getBoolProperty(xProps, "Deletable"))
                        {
                            bSensitiveLibraryModuleDialogDeleteButton = true;
                        }
                        if (getBoolProperty(xProps, "Renamable"))
                        {
                            bSensitiveLibraryModuleDialogRenameButton = true;
                        }
                    }
                }
            }

            // scripts list box state dependant buttons
            weld::TreeView& rScriptsTreeView = m_xScriptsListBox->get_widget();
            std::unique_ptr<weld::TreeIter> xScriptsSelectedIter = rScriptsTreeView.make_iterator();
            if (rScriptsTreeView.n_children()
                && rScriptsTreeView.get_selected(xScriptsSelectedIter.get()))
            {
                bSensitiveAssignButton = true;

                css::uno::Reference<css::script::browse::XBrowseNode> node;
                node = getBrowseNode(rScriptsTreeView, *xScriptsSelectedIter);
                if (node.is())
                {
                    bSensitiveMacroRunButton = true;

                    css::uno::Reference<css::beans::XPropertySet> xProps(node, css::uno::UNO_QUERY);
                    if (xProps.is())
                    {
                        if (getBoolProperty(xProps, "Editable"))
                        {
                            bSensitiveMacroEditButton = true;
                        }
                        if (!bSharedLocationContainer)
                        {
                            if (getBoolProperty(xProps, "Deletable"))
                            {
                                bSensitiveMacroDeleteButton = true;
                            }
                            if (getBoolProperty(xProps, "Renamable"))
                            {
                                bSensitiveMacroRenameButton = true;
                            }
                        }
                    }
                }
            }
        }
    }

    m_xNewLibraryButton->set_sensitive(bSensitiveNewLibraryButton);
    m_xNewModuleButton->set_sensitive(bSensitiveNewModuleButton);
    m_xNewDialogButton->set_sensitive(bSensitiveNewDialogButton);
    m_xLibraryModuleDialogEditButton->set_sensitive(bSensitiveLibraryModuleDialogEditButton);
    m_xLibraryModuleDialogRenameButton->set_sensitive(bSensitiveLibraryModuleDialogRenameButton);
    m_xLibraryModuleDialogDeleteButton->set_sensitive(bSensitiveLibraryModuleDialogDeleteButton);
    m_xLibraryPasswordButton->set_sensitive(bSensitiveLibraryPasswordButton);
    m_xLibraryImportButton->set_sensitive(bSensitiveLibraryImportButton);
    m_xLibraryExportButton->set_sensitive(bSensitiveLibraryExportButton);
    m_xRunButton->set_sensitive(bSensitiveMacroRunButton);
    m_xMacroCreateButton->set_sensitive(bSensitiveMacroCreateButton);
    m_xMacroEditButton->set_sensitive(bSensitiveMacroEditButton);
    m_xMacroRenameButton->set_sensitive(bSensitiveMacroRenameButton);
    m_xMacroDeleteButton->set_sensitive(bSensitiveMacroDeleteButton);
    m_xAssignButton->set_sensitive(bSensitiveAssignButton);
}

// void createLibImpl(weld::Window* pWin, const ScriptDocument& rDocument,
//                    weld::TreeView* pLibBox, SbTreeListBox* pBasicBox)
// basctl/source/basicide/moduldl2.cxx
void MacroManagerDialog::BasicScriptsCreateLibrary(const basctl::ScriptDocument& rDocument)
{
    // create a unique library name
    OUString aLibName = CuiResId(STR_LIBRARY) + OUString::number(1);
    for (sal_uInt32 i = 1; rDocument.hasLibrary(basctl::E_SCRIPTS, aLibName)
                           || rDocument.hasLibrary(basctl::E_DIALOGS, aLibName);
         aLibName = CuiResId(STR_LIBRARY) + OUString::number(++i))
        ;

    InputDialog aInputDlg(m_xDialog.get(), CuiResId(STR_INPUTDIALOG_NEWLIBRARYLABEL));
    aInputDlg.set_title(CuiResId(STR_INPUTDIALOG_NEWLIBRARYTITLE));
    aInputDlg.SetEntryText(aLibName);
    aInputDlg.HideHelpBtn();
    aInputDlg.setCheckEntry([&](OUString sNewName) {
        if (sNewName.isEmpty() || rDocument.hasLibrary(basctl::E_SCRIPTS, sNewName)
            || rDocument.hasLibrary(basctl::E_DIALOGS, sNewName) || sNewName.getLength() > 30
            || !basctl::IsValidSbxName(sNewName))
            return false;
        return true;
    });

    if (!aInputDlg.run())
        return;

    aLibName = aInputDlg.GetEntryText();

    try
    {
        // create library container for modules
        rDocument.getOrCreateLibrary(basctl::E_SCRIPTS, aLibName);

        // create a module
        OUString aModName = rDocument.createObjectName(basctl::E_SCRIPTS, aLibName);
        OUString sModuleCode;
        if (!rDocument.createModule(aLibName, aModName, true, sModuleCode))
            throw css::uno::Exception("could not create module " + aModName, nullptr);

        // tdf#151741 - store all libraries to the file system, otherwise they
        // cannot be renamed/moved since the SfxLibraryContainer::renameLibrary
        // moves the folders/files on the file system
        css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
            rDocument.getLibraryContainer(basctl::E_SCRIPTS));
        css::uno::Reference<css::script::XPersistentLibraryContainer> xModPersLibContainer(
            xModLibContainer, css::uno::UNO_QUERY);
        if (xModPersLibContainer.is())
            xModPersLibContainer->storeLibraries();

        // update Basic IDE object catalog
        SbxItem aModuleSbxItem(SID_BASICIDE_ARG_SBX, rDocument, aLibName, aModName,
                               basctl::SBX_TYPE_MODULE);
        if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
            pDispatcher->ExecuteList(SID_BASICIDE_SBXINSERTED, SfxCallMode::SYNCHRON,
                                     { &aModuleSbxItem });
    }
    catch (const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("basctl.basicide");
    }
}

void MacroManagerDialog::BasicScriptsCreateModule(const basctl::ScriptDocument& rDocument)
{
    // library name is the selected tree entry
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    OUString aLibName = rTreeView.get_text(*xSelectedIter);

    // create an unused module name
    OUString aModName = CuiResId(STR_MODULE) + OUString::number(1);
    for (sal_uInt32 i = 1; rDocument.hasModule(aLibName, aModName);
         aModName = CuiResId(STR_MODULE) + OUString::number(++i))
        ;

    InputDialog aInputDlg(m_xDialog.get(), CuiResId(STR_INPUTDIALOG_NEWMODULELABEL));
    aInputDlg.set_title(CuiResId(STR_INPUTDIALOG_NEWMODULETITLE));
    aInputDlg.SetEntryText(aModName);
    aInputDlg.HideHelpBtn();
    aInputDlg.setCheckEntry([&](OUString sNewName) {
        if (sNewName.isEmpty() || rDocument.hasModule(aLibName, sNewName)
            || sNewName.getLength() > 30 || !basctl::IsValidSbxName(sNewName))
            return false;
        return true;
    });

    if (!aInputDlg.run())
        return;

    aModName = aInputDlg.GetEntryText();

    OUString sModuleCode;
    if (!rDocument.createModule(aLibName, aModName, true /*create main sub*/, sModuleCode))
        return;

    // update Basic IDE object catalog
    SbxItem aSbxItem(SID_BASICIDE_ARG_SBX, rDocument, aLibName, aModName, basctl::SBX_TYPE_MODULE);
    if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
    {
        pDispatcher->ExecuteList(SID_BASICIDE_SBXINSERTED, SfxCallMode::SYNCHRON, { &aSbxItem });
    }
}

void MacroManagerDialog::BasicScriptsCreateDialog(const basctl::ScriptDocument& rDocument)
{
    // library name is the selected tree entry
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    OUString aLibName = rTreeView.get_text(*xSelectedIter);

    // create an unused dialog name
    OUString sDialogName = CuiResId(STR_DIALOG) + OUString::number(1);
    for (sal_uInt32 i = 1; rDocument.hasDialog(aLibName, sDialogName);
         sDialogName = CuiResId(STR_DIALOG) + OUString::number(++i))
        ;

    InputDialog aInputDlg(m_xDialog.get(), CuiResId(STR_INPUTDIALOG_NEWDIALOGLABEL));
    aInputDlg.set_title(CuiResId(STR_INPUTDIALOG_NEWDIALOGTITLE));
    aInputDlg.SetEntryText(sDialogName);
    aInputDlg.HideHelpBtn();
    aInputDlg.setCheckEntry([&](OUString sNewName) {
        if (sNewName.isEmpty() || rDocument.hasDialog(aLibName, sNewName)
            || sNewName.getLength() > 30 || !basctl::IsValidSbxName(sNewName))
            return false;
        return true;
    });

    if (!aInputDlg.run())
        return;

    sDialogName = aInputDlg.GetEntryText();

    try
    {
        rDocument.getOrCreateLibrary(basctl::E_DIALOGS, aLibName);

        css::uno::Reference<css::io::XInputStreamProvider> xISP;
        if (!rDocument.createDialog(aLibName, sDialogName, xISP))
            return;

        // update Basic IDE object catalog
        SbxItem aSbxItem(SID_BASICIDE_ARG_SBX, rDocument, aLibName, sDialogName,
                         basctl::SBX_TYPE_DIALOG);
        if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
        {
            pDispatcher->ExecuteList(SID_BASICIDE_SBXINSERTED, SfxCallMode::SYNCHRON,
                                     { &aSbxItem });
        }
    }
    catch (const css::container::ElementExistException&)
    {
        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(
            m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok,
            CuiResId(STR_SBXNAMEALLREADYUSED)));
        xError->run();
    }
    catch (const css::container::NoSuchElementException&)
    {
        DBG_UNHANDLED_EXCEPTION("basctl.basicide");
    }
}

void MacroManagerDialog::BasicScriptsLibraryModuleDialogEdit(
    const basctl::ScriptDocument& rDocument)
{
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    m_xDialog->hide();

    // show the Basic IDE
    SfxAllItemSet aArgs(SfxGetpApp()->GetPool());
    SfxRequest aRequest(SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs);
    SfxGetpApp()->ExecuteSlot(aRequest);

    if (rTreeView.get_iter_depth(*xSelectedIter) == 2) // library
    {
        SfxUnoAnyItem aDocItem(SID_BASICIDE_ARG_DOCUMENT_MODEL,
                               css::uno::Any(rDocument.getDocumentOrNull()));
        SfxStringItem aLibNameItem(SID_BASICIDE_ARG_LIBNAME,
                                   m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                       ScriptContainerType::LIBRARY));
        if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
            pDispatcher->ExecuteList(SID_BASICIDE_LIBSELECTED, SfxCallMode::ASYNCHRON,
                                     { &aDocItem, &aLibNameItem });
    }
    else // module/dialog
    {
        ScriptContainerInfo* pScriptContainerInfo
            = weld::fromId<ScriptContainerInfo*>(rTreeView.get_id(*xSelectedIter));
        // pScriptContainerInfo->pBrowseNode is nullptr for Dialog entries
        SbxItem aSbxItem(
            SID_BASICIDE_ARG_SBX, rDocument,
            m_xScriptContainersListBox->GetSelectedEntryContainerName(ScriptContainerType::LIBRARY),
            m_xScriptContainersListBox->GetSelectedEntryContainerName(
                ScriptContainerType::MODULEORDIALOG),
            pScriptContainerInfo->pBrowseNode ? basctl::SBX_TYPE_MODULE : basctl::SBX_TYPE_DIALOG);
        if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
            pDispatcher->ExecuteList(SID_BASICIDE_SHOWSBX, SfxCallMode::ASYNCHRON, { &aSbxItem });
    }

    // close the MacroManagerDialog
    m_xDialog->response(0);
}

IMPL_LINK(MacroManagerDialog, CheckPasswordHdl, SvxPasswordDialog*, pDlg, bool)
{
    basctl::ScriptDocument aDocument = m_xScriptContainersListBox->GetScriptDocument();
    if (!aDocument.isAlive())
        return false;

    bool bRet = false;

    css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(
        aDocument.getLibraryContainer(basctl::E_SCRIPTS), css::uno::UNO_QUERY);

    if (xPasswd.is())
    {
        try
        {
            OUString aOldPassword(pDlg->GetOldPassword());
            OUString aNewPassword(pDlg->GetNewPassword());
            OUString aLibName = m_xScriptContainersListBox->GetSelectedEntryContainerName(
                ScriptContainerType::LIBRARY);
            xPasswd->changeLibraryPassword(aLibName, aOldPassword, aNewPassword);
            bRet = true;
        }
        catch (...)
        {
        }
    }

    return bRet;
}

basctl::ScriptDocument ScriptContainersListBox::GetScriptDocument(const weld::TreeIter* pIter)
{
    std::unique_ptr<weld::TreeIter> xIter = m_xTreeView->make_iterator(pIter);
    if (pIter == nullptr)
    {
        if (!m_xTreeView->get_selected(xIter.get()))
            return basctl::ScriptDocument::getApplicationScriptDocument();
    }

    while (m_xTreeView->get_iter_depth(*xIter))
        m_xTreeView->iter_parent(*xIter);

    ScriptContainerInfo* pScriptContainerInfo
        = weld::fromId<ScriptContainerInfo*>(m_xTreeView->get_id(*xIter));
    if (pScriptContainerInfo && pScriptContainerInfo->pBrowseNode)
    {
        css::uno::Reference<css::script::browse::XBrowseNode> aRootNode;
        aRootNode = pScriptContainerInfo->pBrowseNode;
        if (aRootNode->getName() == u"user"_ustr || aRootNode->getName() == u"share"_ustr)
            return basctl::ScriptDocument::getApplicationScriptDocument();
        return basctl::ScriptDocument::getDocumentWithURLOrCaption(aRootNode->getName());
    }

    return basctl::ScriptDocument::getApplicationScriptDocument();
}

IMPL_LINK(MacroManagerDialog, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xCloseButton.get())
    {
        m_xDialog->response(RET_CANCEL);
        return;
    }
    if (&rButton == m_xRunButton.get())
    {
        SaveLastUsedMacro();
        m_xDialog->response(RET_OK);
        return;
    }

    if (m_xScriptContainersListBox->GetSelectedEntryContainerName(ScriptContainerType::LANGUAGE)
        == "Basic")
    {
        basctl::ScriptDocument aDocument = m_xScriptContainersListBox->GetScriptDocument();
        if (!aDocument.isAlive())
            return;

        if (&rButton == m_xNewLibraryButton.get())
        {
            BasicScriptsCreateLibrary(aDocument);
        }
        else if (&rButton == m_xNewModuleButton.get())
        {
            BasicScriptsCreateModule(aDocument);
        }
        else if (&rButton == m_xNewDialogButton.get())
        {
            BasicScriptsCreateDialog(aDocument);
        }
        else if (&rButton == m_xLibraryModuleDialogEditButton.get())
        {
            BasicScriptsLibraryModuleDialogEdit(aDocument);
        }
        else if (&rButton == m_xLibraryModuleDialogRenameButton.get())
        {
            BasicScriptsLibraryModuleDialogRename(aDocument);
        }
        else if (&rButton == m_xLibraryModuleDialogDeleteButton.get())
        {
            BasicScriptsLibraryModuleDialogDelete(aDocument);
        }
        else if (&rButton == m_xLibraryPasswordButton.get())
        {
            BasicScriptsLibraryPassword(aDocument);
        }
        else if (&rButton == m_xLibraryImportButton.get())
        {
            auto insert_entries = [this]() {
                weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
                std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
                if (!rTreeView.get_selected(xSelectedIter.get()))
                    return; // should never happen
                m_xScriptContainersListBox->Fill(xSelectedIter.get());
                rTreeView.expand_row(*xSelectedIter);
                rTreeView.select(*xSelectedIter);
            };
            basctl::ImportLib(aDocument, m_xDialog.get(), {}, {}, insert_entries);
        }
        else if (&rButton == m_xLibraryExportButton.get())
        {
            basctl::Export(aDocument,
                           m_xScriptContainersListBox->GetSelectedEntryContainerName(
                               ScriptContainerType::LIBRARY),
                           m_xDialog.get());
        }
        else if (&rButton == m_xMacroEditButton.get())
        {
            BasicScriptsMacroEdit(aDocument);
        }
        else if (&rButton == m_xMacroDeleteButton.get())
        {
            // todo
            // see: void MacroChooser::DeleteMacro()
            return;
        }
        else if (&rButton == m_xAssignButton.get())
        {
            SfxAllItemSet Args(SfxGetpApp()->GetPool());
            SfxAllItemSet aInternalSet(SfxGetpApp()->GetPool());
            if (m_xDocumentFrame.is())
                aInternalSet.Put(SfxUnoFrameItem(SID_FILLFRAME, m_xDocumentFrame));
            SfxRequest aRequest(SID_CONFIGACCEL, SfxCallMode::SYNCHRON, Args, aInternalSet);

            SfxMacroInfoItem aMacroInfoItem(
                SID_MACROINFO, aDocument.getBasicManager(),
                m_xScriptContainersListBox->GetSelectedEntryContainerName(
                    ScriptContainerType::LIBRARY),
                m_xScriptContainersListBox->GetSelectedEntryContainerName(
                    ScriptContainerType::MODULEORDIALOG),
                m_xScriptsListBox->GetSelectedScriptName(), OUString(),
                m_xScriptContainersListBox->GetSelectedEntryContainerName(
                    ScriptContainerType::LOCATION));
            aRequest.AppendItem(aMacroInfoItem);

            SfxGetpApp()->ExecuteSlot(aRequest);
        }
        return;
    }

    //
    // button operations on Scripting Framework languages
    //
    if (&rButton == m_xNewLibraryButton.get())
    {
        ScriptingFrameworkScriptsCreateEntry(InputDialogMode::NEWLIB);
    }
    else if (&rButton == m_xMacroCreateButton.get())
    {
        ScriptingFrameworkScriptsCreateEntry(InputDialogMode::NEWMACRO);
    }
    else if (&rButton == m_xMacroEditButton.get())
    {
        weld::TreeView& rTreeView = m_xScriptsListBox->get_widget();
        std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
        if (!rTreeView.get_selected(xSelectedIter.get()))
            return; // should never happen
        css::uno::Reference<css::script::browse::XBrowseNode> node
            = getBrowseNode(rTreeView, *xSelectedIter);
        css::uno::Reference<css::script::XInvocation> xInv(node, css::uno::UNO_QUERY);
        if (xInv.is())
        {
            m_xDialog->response(RET_CANCEL);
            css::uno::Sequence<css::uno::Any> args(0);
            css::uno::Sequence<css::uno::Any> outArgs(0);
            css::uno::Sequence<sal_Int16> outIndex;
            try
            {
                // ISSUE need code to run script here
                xInv->invoke(u"Editable"_ustr, args, outIndex, outArgs);
            }
            catch (css::uno::Exception const&)
            {
                TOOLS_WARN_EXCEPTION("cui.dialogs", "Caught exception trying to invoke");
            }
        }
    }
    else if (&rButton == m_xLibraryModuleDialogDeleteButton.get())
    {
        weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
        std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
        if (!rTreeView.get_selected(xSelectedIter.get()))
            return; // should never happen
        ScriptingFrameworkScriptsDeleteEntry(rTreeView, *xSelectedIter);
    }
    else if (&rButton == m_xMacroDeleteButton.get())
    {
        weld::TreeView& rTreeView = m_xScriptsListBox->get_widget();
        std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
        if (!rTreeView.get_selected(xSelectedIter.get()))
            return; // should never happen
        ScriptingFrameworkScriptsDeleteEntry(rTreeView, *xSelectedIter);
    }
    else if (&rButton == m_xLibraryModuleDialogRenameButton.get())
    {
        weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
        std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
        if (!rTreeView.get_selected(xSelectedIter.get()))
            return; // should never happen
        ScriptingFrameworkScriptsRenameEntry(rTreeView, *xSelectedIter);
    }
    else if (&rButton == m_xMacroRenameButton.get())
    {
        weld::TreeView& rTreeView = m_xScriptsListBox->get_widget();
        std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
        if (!rTreeView.get_selected(xSelectedIter.get()))
            return; // should never happen
        ScriptingFrameworkScriptsRenameEntry(rTreeView, *xSelectedIter);
    }
    else if (&rButton == m_xAssignButton.get())
    {
        SfxAllItemSet Args(SfxGetpApp()->GetPool());
        SfxAllItemSet aInternalSet(SfxGetpApp()->GetPool());
        if (m_xDocumentFrame.is())
            aInternalSet.Put(SfxUnoFrameItem(SID_FILLFRAME, m_xDocumentFrame));
        SfxRequest aRequest(SID_CONFIGACCEL, SfxCallMode::SYNCHRON, Args, aInternalSet);

        SfxMacroInfoItem aMacroInfoItem(
            SID_MACROINFO, nullptr,
            m_xScriptContainersListBox->GetSelectedEntryContainerName(ScriptContainerType::LIBRARY),
            OUString(), m_xScriptsListBox->GetSelectedScriptName(), OUString(),
            m_xScriptContainersListBox->GetSelectedEntryContainerName(
                ScriptContainerType::LOCATION));
        aRequest.AppendItem(aMacroInfoItem);
        SfxGetpApp()->ExecuteSlot(aRequest);
    }
}

bool MacroManagerDialog::IsLibraryReadOnlyOrFailedPasswordQuery(
    const basctl::ScriptDocument& rDocument, weld::TreeIter* pIter)
{
    css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
        rDocument.getLibraryContainer(basctl::E_SCRIPTS));
    css::uno::Reference<css::script::XLibraryContainer2> xDlgLibContainer(
        rDocument.getLibraryContainer(basctl::E_DIALOGS));

    OUString aLibName
        = m_xScriptContainersListBox->GetContainerName(*pIter, ScriptContainerType::LIBRARY);

    // check if library is readonly
    if ((xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
         && xModLibContainer->isLibraryReadOnly(aLibName)
         && !xModLibContainer->isLibraryLink(aLibName))
        || (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
            && xDlgLibContainer->isLibraryReadOnly(aLibName)
            && !xDlgLibContainer->isLibraryLink(aLibName)))
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(
            Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                                             VclButtonsType::Ok, CuiResId(STR_LIBISREADONLY)));
        xErrorBox->run();
        return true;
    }

    // password
    if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
        /*&& !xModLibContainer->isLibraryLoaded(aLibName)*/)
    {
        bool bOK = true;
        // check password
        css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(xModLibContainer,
                                                                            css::uno::UNO_QUERY);
        if (xPasswd.is() && xPasswd->isLibraryPasswordProtected(aLibName)
            && !xPasswd->isLibraryPasswordVerified(aLibName))
        {
            OUString sPassword;
            bOK = basctl::QueryPassword(m_xDialog.get(), xModLibContainer, aLibName, sPassword,
                                        true, true);
        }
        if (!bOK)
            return true;
    }

    return false;
}

// adapted from LibPage inline renaming
// IMPL_LINK(LibPage, EditingEntryHdl, const weld::TreeIter&, rIter, bool)
// IMPL_LINK(LibPage, EditedEntryHdl, const IterString&, rIterString, bool)
// basctl/source/basicide/moduldl2.cxx
void MacroManagerDialog::BasicScriptsLibraryModuleDialogRename(
    const basctl::ScriptDocument& rDocument)
{
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    if (IsLibraryReadOnlyOrFailedPasswordQuery(rDocument, xSelectedIter.get()))
        return;

    OUString sOldName = rTreeView.get_text(*xSelectedIter);

    if (rTreeView.get_iter_depth(*xSelectedIter) == 2) // library
    {
        InputDialog aInputDlg(m_xDialog.get(), CuiResId(STR_INPUTDIALOG_RENAMELIBRARYLABEL));
        aInputDlg.HideHelpBtn();
        aInputDlg.set_title(CuiResId(STR_INPUTDIALOG_RENAMELIBRARYTITLE));
        aInputDlg.SetEntryText(sOldName);
        aInputDlg.setCheckEntry([&](OUString sNewName) {
            if (sNewName != sOldName
                && (sNewName.isEmpty() || rDocument.hasLibrary(basctl::E_SCRIPTS, sNewName)
                    || rDocument.hasLibrary(basctl::E_DIALOGS, sNewName)
                    || sNewName.getLength() > 30 || !basctl::IsValidSbxName(sNewName)))
                return false;
            return true;
        });

        if (!aInputDlg.run())
            return;

        OUString sNewName = aInputDlg.GetEntryText();
        if (sNewName == sOldName)
            return;

        bool bSuccess = true;
        try
        {
            css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
                rDocument.getLibraryContainer(basctl::E_SCRIPTS));
            if (xModLibContainer.is() && xModLibContainer->hasByName(sOldName))
                xModLibContainer->renameLibrary(sOldName, sNewName);
            css::uno::Reference<css::script::XLibraryContainer2> xDlgLibContainer(
                rDocument.getLibraryContainer(basctl::E_DIALOGS));
            if (xDlgLibContainer.is() && xDlgLibContainer->hasByName(sOldName))
                xDlgLibContainer->renameLibrary(sOldName, sNewName);
        }
        catch (css::uno::Exception const&)
        {
            bSuccess = false;
            TOOLS_WARN_EXCEPTION("cui.dialogs", "Caught exception trying to Rename");
        }

        if (bSuccess)
        {
            basctl::MarkDocumentModified(rDocument);
            //            rTreeView.set_text(*xSelectedIter, sNewName);
        }
    }
    else
    {
        // module/dialog
        ScriptContainerInfo* pScriptContainerInfo
            = weld::fromId<ScriptContainerInfo*>(rTreeView.get_id(*xSelectedIter));
        if (!pScriptContainerInfo)
            return; // should never happen

        OUString aLibName = m_xScriptContainersListBox->GetSelectedEntryContainerName(
            ScriptContainerType::LIBRARY);

        InputDialog aInputDlg(m_xDialog.get(), CuiResId(pScriptContainerInfo->pBrowseNode
                                                            ? STR_INPUTDIALOG_RENAMEMODULELABEL
                                                            : STR_INPUTDIALOG_RENAMEDIALOGLABEL));
        aInputDlg.HideHelpBtn();
        aInputDlg.set_title(CuiResId(pScriptContainerInfo->pBrowseNode
                                         ? STR_INPUTDIALOG_RENAMEMODULETITLE
                                         : STR_INPUTDIALOG_RENAMEMODULETITLE));
        aInputDlg.SetEntryText(sOldName);
        aInputDlg.setCheckEntry([&](OUString sNewName) {
            if (sNewName != sOldName
                && (sNewName.isEmpty() || sNewName.getLength() > 30
                            || !basctl::IsValidSbxName(sNewName)
                            || pScriptContainerInfo->pBrowseNode
                        ? rDocument.hasModule(aLibName, sNewName)
                        : rDocument.hasDialog(aLibName, sNewName)))
                return false;
            return true;
        });

        if (!aInputDlg.run())
            return;

        OUString sNewName = aInputDlg.GetEntryText();
        if (sNewName == sOldName)
            return;

        if (pScriptContainerInfo->pBrowseNode
                ? rDocument.renameModule(aLibName, sOldName, sNewName)
                : rDocument.renameDialog(aLibName, sOldName, sNewName, nullptr))
        {
            basctl::MarkDocumentModified(rDocument);
            //            rTreeView.set_text(*xSelectedIter, sNewName);
        }
    }
}

void MacroManagerDialog::BasicScriptsLibraryModuleDialogDelete(
    const basctl::ScriptDocument& rDocument)
{
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    if (IsLibraryReadOnlyOrFailedPasswordQuery(rDocument, xSelectedIter.get()))
        return;

    if (rTreeView.get_iter_depth(*xSelectedIter) == 2) // library
    {
        // see: void LibPage::DeleteCurrent()
        // basctl/source/basicide/moduldl2.cxx
        OUString aLibName = rTreeView.get_text(*xSelectedIter);

        // check, if library is link
        bool bIsLibraryLink = false;
        css::uno::Reference<css::script::XLibraryContainer2> xModLibContainer(
            rDocument.getLibraryContainer(basctl::E_SCRIPTS));
        css::uno::Reference<css::script::XLibraryContainer2> xDlgLibContainer(
            rDocument.getLibraryContainer(basctl::E_DIALOGS));
        if ((xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
             && xModLibContainer->isLibraryLink(aLibName))
            || (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName)
                && xDlgLibContainer->isLibraryLink(aLibName)))
        {
            bIsLibraryLink = true;
        }

        if (!basctl::QueryDelLib(aLibName, bIsLibraryLink, m_xDialog.get()))
            return;

        // inform BasicIDE
        SfxUnoAnyItem aDocItem(SID_BASICIDE_ARG_DOCUMENT_MODEL,
                               css::uno::Any(rDocument.getDocumentOrNull()));
        SfxStringItem aLibNameItem(SID_BASICIDE_ARG_LIBNAME, aLibName);
        if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
            pDispatcher->ExecuteList(SID_BASICIDE_LIBREMOVED, SfxCallMode::SYNCHRON,
                                     { &aDocItem, &aLibNameItem });

        // remove library from module and dialog library containers
        if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName))
            xModLibContainer->removeLibrary(aLibName);
        if (xDlgLibContainer.is() && xDlgLibContainer->hasByName(aLibName))
            xDlgLibContainer->removeLibrary(aLibName);

        basctl::MarkDocumentModified(rDocument);
    }
    else
    {
        // delete Basic module or dialog
        // adapted from void ObjectPage::DeleteCurrent() basctl/source/basicide/moduldlg.cxx
        ScriptContainerInfo* pScriptContainerInfo
            = weld::fromId<ScriptContainerInfo*>(rTreeView.get_id(*xSelectedIter));
        if (!pScriptContainerInfo)
            return; // shouldn't happen

        // pBrowseNode points to nullptr for dialog entries
        if (pScriptContainerInfo->pBrowseNode)
        {
            if (!basctl::QueryDelModule(m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                            ScriptContainerType::MODULEORDIALOG),
                                        m_xDialog.get()))
                return;
        }
        else
        {
            if (!basctl::QueryDelDialog(m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                            ScriptContainerType::MODULEORDIALOG),
                                        m_xDialog.get()))
                return;
        }

        bool bSuccess = false;
        if (pScriptContainerInfo->pBrowseNode)
            bSuccess
                = rDocument.removeModule(m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                             ScriptContainerType::LIBRARY),
                                         m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                             ScriptContainerType::MODULEORDIALOG));
        else
            bSuccess
                = rDocument.removeDialog(m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                             ScriptContainerType::LIBRARY),
                                         m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                             ScriptContainerType::MODULEORDIALOG));

        if (bSuccess)
        {
            if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
            {
                SbxItem aSbxItem(SID_BASICIDE_ARG_SBX, rDocument,
                                 m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                     ScriptContainerType::LIBRARY),
                                 m_xScriptContainersListBox->GetSelectedEntryContainerName(
                                     ScriptContainerType::MODULEORDIALOG),
                                 pScriptContainerInfo->pBrowseNode ? basctl::SBX_TYPE_MODULE
                                                                   : basctl::SBX_TYPE_DIALOG);
                pDispatcher->ExecuteList(SID_BASICIDE_SBXDELETED, SfxCallMode::SYNCHRON,
                                         { &aSbxItem });
            }
            basctl::MarkDocumentModified(rDocument);
        }
    }
}

// inspired by IMPL_LINK( LibPage, ButtonHdl, weld::Button&, rButton, void )
// else if (&rButton == m_xPasswordButton.get())
// basctl/source/basicide/moduldl2.cxx
void MacroManagerDialog::BasicScriptsLibraryPassword(const basctl::ScriptDocument& rDocument)
{
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    OUString aLibName = rTreeView.get_text(*xSelectedIter);

    // load module library (if not loaded)
    css::uno::Reference<css::script::XLibraryContainer> xModLibContainer
        = rDocument.getLibraryContainer(basctl::E_SCRIPTS);
    if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName)
        && !xModLibContainer->isLibraryLoaded(aLibName))
    {
        xModLibContainer->loadLibrary(aLibName);
    }

    // check if library is password protected --> this is for setting and removing password
    if (xModLibContainer.is() && xModLibContainer->hasByName(aLibName))
    {
        css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(xModLibContainer,
                                                                            css::uno::UNO_QUERY);
        if (xPasswd.is())
        {
            if (xPasswd->isLibraryPasswordProtected(aLibName)
                && !xPasswd->isLibraryPasswordVerified(aLibName))
            {
                // password not verified
                OUString sPassword;
                if (basctl::QueryPassword(m_xDialog.get(), xModLibContainer, aLibName, sPassword,
                                          true, true))
                {
                    CheckButtons();
                }
                return;
            }

            // set/change password dialog
            SvxPasswordDialog aDlg(m_xDialog.get(), !xPasswd->isLibraryPasswordProtected(aLibName));
            aDlg.SetCheckPasswordHdl(LINK(this, MacroManagerDialog, CheckPasswordHdl));

            if (aDlg.run() == RET_OK)
            {
                if (xPasswd->isLibraryPasswordProtected(aLibName))
                    rTreeView.set_image(*xSelectedIter, RID_CUIBMP_LOCKED);
                else
                    rTreeView.set_image(*xSelectedIter, RID_CUIBMP_LIB);
                basctl::MarkDocumentModified(rDocument);
            }

            rTreeView.grab_focus();
        }
    }
}

void MacroManagerDialog::BasicScriptsMacroEdit(const basctl::ScriptDocument& rDocument)
{
    // hide the scripts organizer selector dialog before opening the basic ide
    m_xDialog->hide();

    // open the basic ide
    SfxAllItemSet aArgs(SfxGetpApp()->GetPool());
    SfxRequest aRequest(SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs);
    SfxGetpApp()->ExecuteSlot(aRequest);

    // navigate to the method
    SbxItem aSbxItem(
        SID_BASICIDE_ARG_SBX, rDocument,
        m_xScriptContainersListBox->GetSelectedEntryContainerName(ScriptContainerType::LIBRARY),
        m_xScriptContainersListBox->GetSelectedEntryContainerName(
            ScriptContainerType::MODULEORDIALOG),
        m_xScriptsListBox->GetSelectedScriptName(), basctl::SBX_TYPE_METHOD);
    // need to use basctl::GetDispatcher() to have expected results with qt5 and x11,
    // gtk3 works with SfxGetpApp()->GetDispatcher()
    if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
        pDispatcher->ExecuteList(SID_BASICIDE_SHOWSBX, SfxCallMode::ASYNCHRON, { &aSbxItem });
    // alternate way to do this
    // BasicManager* pBasMgr = rDocument.getBasicManager();
    // SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLibName, aModName, aMethodName, OUString() );
    // if (SfxDispatcher* pDispatcher = basctl::GetDispatcher())
    //     pDispatcher->ExecuteList(SID_BASICIDE_EDITMACRO, SfxCallMode::ASYNCHRON, { &aInfoItem });

    // now it is safe to close the scripts organizer selector dialog
    m_xDialog->response(0);
}

// modified version of void SvxScriptOrgDialog::renameEntry(const weld::TreeIter& rEntry)
// cui/source/dialogs/scriptdlg.cxx
void MacroManagerDialog::ScriptingFrameworkScriptsRenameEntry(weld::TreeView& rTreeView,
                                                              const weld::TreeIter& rEntry)
{
    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode
        = getBrowseNode(rTreeView, rEntry);
    css::uno::Reference<css::script::XInvocation> xInv(xBrowseNode, css::uno::UNO_QUERY);

    if (xInv.is())
    {
        OUString aNewName = xBrowseNode->getName();
        sal_Int32 extnPos = aNewName.lastIndexOf('.');
        if (extnPos > 0)
        {
            aNewName = aNewName.copy(0, extnPos);
        }

        InputDialog aInputDlg(m_xDialog.get(),
                              xBrowseNode->getType()
                                      == css::script::browse::BrowseNodeTypes::CONTAINER
                                  ? CuiResId(STR_INPUTDIALOG_RENAMELIBRARYLABEL)
                                  : CuiResId(STR_INPUTDIALOG_RENAMEMACROLABEL));
        aInputDlg.set_title(xBrowseNode->getType()
                                    == css::script::browse::BrowseNodeTypes::CONTAINER
                                ? CuiResId(STR_INPUTDIALOG_RENAMELIBRARYTITLE)
                                : CuiResId(STR_INPUTDIALOG_RENAMEMACROTITLE));
        aInputDlg.SetEntryText(aNewName);
        aInputDlg.HideHelpBtn();
        // doesn't check if the name already exists, that will be caught below by invoke
        aInputDlg.setCheckEntry([](OUString sNewName) {
            if (sNewName.isEmpty() || sNewName.getLength() > 30
                || !basctl::IsValidSbxName(sNewName))
                return false;
            return true;
        });

        if (!aInputDlg.run())
            return;

        aNewName = aInputDlg.GetEntryText();

        css::uno::Sequence<css::uno::Any> args{ css::uno::Any(aNewName) };
        css::uno::Sequence<css::uno::Any> outArgs;
        css::uno::Sequence<sal_Int16> outIndex;
        try
        {
            css::uno::Any aResult = xInv->invoke(u"Renamable"_ustr, args, outIndex, outArgs);
            xBrowseNode.set(aResult, css::uno::UNO_QUERY);
        }
        catch (css::uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("cui.dialogs", "Caught exception trying to Rename");
        }
    }
    if (xBrowseNode.is())
    {
        if (xBrowseNode->getType() == css::script::browse::BrowseNodeTypes::CONTAINER)
        {
            m_xScriptContainersListBox->ScriptContainerSelected();
        }
        else
        {
            ScriptInfo* pScriptInfo = weld::fromId<ScriptInfo*>(m_xScriptsListBox->get_id(rEntry));
            if (pScriptInfo)
            {
                css::uno::Reference<css::beans::XPropertySet> xPropSet(xBrowseNode,
                                                                       css::uno::UNO_QUERY);
                if (xPropSet.is())
                {
                    OUString sURI;
                    try
                    {
                        css::uno::Any value = xPropSet->getPropertyValue(u"URI"_ustr);
                        value >>= sURI;
                    }
                    catch (css::uno::Exception&)
                    {
                        // do nothing, the URI will be empty
                    }
                    pScriptInfo->sURL = sURI;
                }
            }
        }
        rTreeView.set_text(rEntry, xBrowseNode->getName());
        rTreeView.set_cursor(rEntry);
    }
    else
    {
        //ISSUE L10N & message from exception?
        OUString aError(CuiResId(RID_CUISTR_RENAMEFAILED));
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(
            m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok, aError));
        xErrorBox->set_title(CuiResId(RID_CUISTR_RENAMEFAILED_TITLE));
        xErrorBox->run();
    }
}

// for Scripting Framework entries
// duplicate of OUString SvxScriptOrgDialog::getListOfChildren
// cui/source/dialogs/scriptdlg.cxx
OUString MacroManagerDialog::getListOfChildren(
    const css::uno::Reference<css::script::browse::XBrowseNode>& node, int depth)
{
    OUStringBuffer result = "\n";
    for (int i = 0; i <= depth; i++)
    {
        result.append("\t");
    }
    result.append(node->getName());

    try
    {
        if (node->hasChildNodes())
        {
            const css::uno::Sequence<css::uno::Reference<css::script::browse::XBrowseNode>> children
                = node->getChildNodes();
            for (const css::uno::Reference<css::script::browse::XBrowseNode>& n : children)
            {
                result.append(getListOfChildren(n, depth + 1));
            }
        }
    }
    catch (css::uno::Exception&)
    {
        // ignore, will return an empty string
    }

    return result.makeStringAndClear();
}

// modified verson of void SvxScriptOrgDialog::deleteEntry(const weld::TreeIter& rEntry)
// cui/source/dialogs/scriptdlg.cxx
void MacroManagerDialog::ScriptingFrameworkScriptsDeleteEntry(weld::TreeView& rTreeView,
                                                              const weld::TreeIter& rEntry)
{
    bool result = false;
    css::uno::Reference<css::script::browse::XBrowseNode> node = getBrowseNode(rTreeView, rEntry);
    // ISSUE L10N string & can we center list?
    OUString aQuery = CuiResId(RID_CUISTR_DELQUERY) + getListOfChildren(node, 0);
    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Question, VclButtonsType::YesNo, aQuery));
    xQueryBox->set_title(CuiResId(RID_CUISTR_DELQUERY_TITLE));
    if (xQueryBox->run() == RET_NO)
    {
        return;
    }

    css::uno::Reference<css::script::XInvocation> xInv(node, css::uno::UNO_QUERY);
    if (xInv.is())
    {
        css::uno::Sequence<css::uno::Any> args(0);
        css::uno::Sequence<css::uno::Any> outArgs(0);
        css::uno::Sequence<sal_Int16> outIndex;
        try
        {
            css::uno::Any aResult = xInv->invoke(u"Deletable"_ustr, args, outIndex, outArgs);
            aResult >>= result; // or do we just assume true if no exception ?
        }
        catch (css::uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("cui.dialogs", "Caught exception trying to delete");
        }
    }
    if (result)
    {
        if (&rTreeView == &m_xScriptContainersListBox->get_widget())
        {
            m_xScriptContainersListBox->Remove(&rEntry, true);
            SelectHdl(rTreeView);
        }
        else
        {
            m_xScriptsListBox->Remove(rEntry);
        }
        UpdateUI();
    }
    else
    {
        //ISSUE L10N & message from exception?
        std::unique_ptr<weld::MessageDialog> xErrorBox(
            Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                                             VclButtonsType::Ok, CuiResId(RID_CUISTR_DELFAILED)));
        xErrorBox->set_title(CuiResId(RID_CUISTR_CREATEFAILED_TITLE));
        xErrorBox->run();
    }
}

// Modified version of SvxScriptOrgDialog::CreateEntry
// cui/source/dialogs/scriptdlg.cxx
void MacroManagerDialog::ScriptingFrameworkScriptsCreateEntry(InputDialogMode eInputDialogMode)
{
    weld::TreeView& rTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xSelectedIter = rTreeView.make_iterator();
    if (!rTreeView.get_selected(xSelectedIter.get()))
        return; // should never happen

    css::uno::Reference<css::script::browse::XBrowseNode> aChildNode;
    css::uno::Reference<css::script::browse::XBrowseNode> xBrowseNode
        = getBrowseNode(rTreeView, *xSelectedIter);
    css::uno::Reference<css::script::XInvocation> xInv(xBrowseNode, css::uno::UNO_QUERY);

    // Currently, invocation is not implemented for python, only beanshell, javascript, and java.
    if (xInv.is())
    {
        OUString aNewName;
        OUString aNewStdName;
        if (eInputDialogMode == InputDialogMode::NEWLIB)
        {
            aNewStdName = CuiResId(STR_LIBRARY);
        }
        else
        {
            aNewStdName = CuiResId(STR_MACRO);
        }

        bool bValid = false;
        sal_Int32 i = 1;

        css::uno::Sequence<css::uno::Reference<css::script::browse::XBrowseNode>> childNodes;
        // no children => ok to create Parcel1 or Script1 without checking ?
        try
        {
            if (!xBrowseNode->hasChildNodes())
            {
                aNewName = aNewStdName + OUString::number(i);
                bValid = true;
            }
            else
            {
                childNodes = xBrowseNode->getChildNodes();
            }
        }
        catch (css::uno::Exception&)
        {
            // ignore, will continue on with empty sequence
        }

        OUString extn;
        while (!bValid)
        {
            aNewName = aNewStdName + OUString::number(i);
            bool bFound = false;
            if (childNodes.hasElements())
            {
                OUString nodeName = childNodes[0]->getName();
                sal_Int32 extnPos = nodeName.lastIndexOf('.');
                if (extnPos > 0)
                    extn = nodeName.copy(extnPos);
            }
            for (const css::uno::Reference<css::script::browse::XBrowseNode>& n : childNodes)
            {
                if (Concat2View(aNewName + extn) == n->getName())
                {
                    bFound = true;
                    break;
                }
            }
            if (bFound)
            {
                i++;
            }
            else
            {
                bValid = true;
            }
        }

        InputDialog aInputDlg(m_xDialog.get(), eInputDialogMode == InputDialogMode::NEWLIB
                                                   ? CuiResId(STR_INPUTDIALOG_NEWLIBRARYLABEL)
                                                   : CuiResId(STR_INPUTDIALOG_NEWMACROLABEL));
        aInputDlg.set_title(eInputDialogMode == InputDialogMode::NEWLIB
                                ? CuiResId(STR_INPUTDIALOG_NEWLIBRARYTITLE)
                                : CuiResId(STR_INPUTDIALOG_NEWMACROTITLE));
        aInputDlg.SetEntryText(aNewName);
        aInputDlg.HideHelpBtn();

        // setCheckEntry doesn't check if the name already exists. It is checked after the dialog
        // in the Creatable invocation call - this could be improved by including a check for
        // existing name
        aInputDlg.setCheckEntry([](OUString sNewName) {
            if (sNewName.isEmpty() || sNewName.getLength() > 30
                || !basctl::IsValidSbxName(sNewName))
                return false;
            return true;
        });

        do
        {
            if (aInputDlg.run())
            {
                OUString aUserSuppliedName = aInputDlg.GetEntryText();
                bValid = true;
                for (const css::uno::Reference<css::script::browse::XBrowseNode>& n : childNodes)
                {
                    if (Concat2View(aUserSuppliedName + extn) == n->getName())
                    {
                        bValid = false;
                        OUString aError = CuiResId(RID_CUISTR_CREATEFAILED)
                                          + CuiResId(RID_CUISTR_CREATEFAILEDDUP);

                        std::unique_ptr<weld::MessageDialog> xErrorBox(
                            Application::CreateMessageDialog(m_xDialog.get(),
                                                             VclMessageType::Warning,
                                                             VclButtonsType::Ok, aError));
                        xErrorBox->set_title(CuiResId(RID_CUISTR_CREATEFAILED_TITLE));
                        xErrorBox->run();
                        aInputDlg.SetEntryText(aNewName);
                        break;
                    }
                }
                if (bValid)
                    aNewName = aUserSuppliedName;
            }
            else
            {
                // user hit cancel
                return;
            }
        } while (!bValid);

        // open up parent node (which ensures it's loaded)
        rTreeView.expand_row(*xSelectedIter);

        css::uno::Sequence<css::uno::Any> args{ css::uno::Any(aNewName) };
        css::uno::Sequence<css::uno::Any> outArgs;
        css::uno::Sequence<sal_Int16> outIndex;
        try
        {
            css::uno::Any aResult = xInv->invoke(u"Creatable"_ustr, args, outIndex, outArgs);
            aChildNode.set(aResult, css::uno::UNO_QUERY);
        }
        catch (css::uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("cui.dialogs", "Caught exception trying to Create");
        }
    }
    if (aChildNode.is())
    {
        if (rTreeView.get_iter_depth(*xSelectedIter) == 1) // language node
        {
            m_xScriptContainersListBox->Fill(xSelectedIter.get());
            rTreeView.expand_row(*xSelectedIter);
            rTreeView.select(*xSelectedIter);
        }
        else
        {
            m_xScriptContainersListBox->ScriptContainerSelected();
        }
        UpdateUI();
    }
    else
    {
        //ISSUE L10N & message from exception?
        OUString aError(CuiResId(RID_CUISTR_CREATEFAILED));
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(
            m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok, aError));
        xErrorBox->set_title(CuiResId(RID_CUISTR_CREATEFAILED_TITLE));
        xErrorBox->run();
    }
}

// called from OUString AbstractMacroManagerDialog_Impl::GetScriptURL() const
// cui/source/factory/dlgfact.cxx
OUString MacroManagerDialog::GetScriptURL() const
{
    OUString result;
    std::unique_ptr<weld::TreeIter> xIter = m_xScriptsListBox->make_iterator();
    if (m_xScriptsListBox->get_selected(xIter.get()))
    {
        ScriptInfo* pScriptInfo = weld::fromId<ScriptInfo*>(m_xScriptsListBox->get_id(*xIter));
        if (pScriptInfo)
            result = pScriptInfo->sURL;
    }
    return result;
}

constexpr OUString MACRO_MANAGER_CONFIGNAME = u"MacroManagerDialog"_ustr;
constexpr OUString LAST_RUN_MACRO_INFO = u"LastRunMacro"_ustr;

// adapted from SvxScriptSelectorDialog::SaveLastUsedMacro()
// cui/source/customize/cfgutil.cxx
void MacroManagerDialog::SaveLastUsedMacro()
{
    // Gets the current selection in the dialog as a series of selected entries
    OUString sMacroInfo = m_xScriptsListBox->GetSelectedScriptName();

    weld::TreeView& rScriptContainersTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xIter = rScriptContainersTreeView.make_iterator();

    if (!rScriptContainersTreeView.get_selected(xIter.get()))
        return;

    do
    {
        sMacroInfo = rScriptContainersTreeView.get_text(*xIter) + "|" + sMacroInfo;
    } while (rScriptContainersTreeView.iter_parent(*xIter));

    SvtViewOptions(EViewType::Dialog, MACRO_MANAGER_CONFIGNAME)
        .SetUserItem(LAST_RUN_MACRO_INFO, css::uno::Any(sMacroInfo));
}

// adapted from SvxScriptSelectorDialog::LoadLastUsedMacro()
// cui/source/customize/cfgutil.cxx
void MacroManagerDialog::LoadLastUsedMacro()
{
    SvtViewOptions aDlgOpt(EViewType::Dialog, MACRO_MANAGER_CONFIGNAME);
    if (!aDlgOpt.Exists())
        return;

    OUString sMacroInfo;
    aDlgOpt.GetUserItem(LAST_RUN_MACRO_INFO) >>= sMacroInfo;
    if (sMacroInfo.isEmpty())
        return;

    // Counts how many entries exist in the macro info string
    sal_Int16 nInfoParts = 0;
    sal_Int16 nLastIndex = sMacroInfo.indexOf('|');
    if (nLastIndex > -1)
    {
        nInfoParts = 1;
        while (nLastIndex != -1)
        {
            nInfoParts++;
            nLastIndex = sMacroInfo.indexOf('|', nLastIndex + 1);
        }
    }

    weld::TreeView& rScriptContainersTreeView = m_xScriptContainersListBox->get_widget();
    std::unique_ptr<weld::TreeIter> xIter = rScriptContainersTreeView.make_iterator();

    if (!rScriptContainersTreeView.get_iter_first(*xIter))
        return;

    // Expand the nodes in the script containers tree
    OUString sNodeToExpand;
    bool bIsIterValid;
    sal_Int16 nOpenedNodes = 0;
    for (sal_Int16 i = 0; i < nInfoParts - 1; i++)
    {
        sNodeToExpand = sMacroInfo.getToken(i, '|');
        bIsIterValid = true;
        while (bIsIterValid && rScriptContainersTreeView.get_text(*xIter) != sNodeToExpand)
            bIsIterValid = rScriptContainersTreeView.iter_next_sibling(*xIter);

        if (bIsIterValid)
        {
            rScriptContainersTreeView.expand_row(*xIter);
            nOpenedNodes++;
        }
        if (rScriptContainersTreeView.iter_has_child(*xIter))
            (void)rScriptContainersTreeView.iter_children(*xIter);
        else if (nOpenedNodes < nInfoParts - 1)
            // If the number of levels in the tree is smaller than the
            // number of parts in the macro info string, then return
            return;
    }
    rScriptContainersTreeView.select(*xIter);
    rScriptContainersTreeView.scroll_to_row(*xIter);

    // fill the scripts list box and set the label
    m_xScriptContainersListBox->ScriptContainerSelected();
    m_xScriptsListBoxLabel->set_label(m_aScriptsListBoxLabelBaseStr + " "
                                      + rScriptContainersTreeView.get_text(*xIter));

    // Select the macro in the scripts tree
    weld::TreeView& rScriptsTreeView = m_xScriptsListBox->get_widget();
    xIter = rScriptsTreeView.make_iterator();
    if (rScriptsTreeView.get_iter_first(*xIter))
    {
        OUString sMacroName = sMacroInfo.getToken(nInfoParts - 1, '|');
        bIsIterValid = true;
        while (bIsIterValid && rScriptsTreeView.get_text(*xIter) != sMacroName)
            bIsIterValid = rScriptsTreeView.iter_next_sibling(*xIter);
        if (bIsIterValid)
        {
            rScriptsTreeView.scroll_to_row(*xIter);
            rScriptsTreeView.select(*xIter);
        }
    }

    UpdateUI();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
