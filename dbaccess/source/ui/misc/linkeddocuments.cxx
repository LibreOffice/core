/*************************************************************************
 *
 *  $RCSfile: linkeddocuments.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#include "linkeddocuments.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#include <com/sun/star/task/XJobExecutor.hpp>
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _DBAUI_FILENOTATION_HXX_
#include "filenotation.hxx"
#endif
#ifndef _DBAUI_MISSINGDOCDLG_HXX_
#include "missingdocdlg.hxx"
#endif
#ifndef _DBAUI_DOCLINKDIALOG_HXX_
#include "doclinkdialog.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _SFXNEW_HXX
#include <sfx2/new.hxx>
#endif
#ifndef _DBAUI_DOCUMENTAUTOLINKER_HXX_
#include "documentautolinker.hxx"
#endif
#ifndef _SVTOOLS_TEMPLDLG_HXX
#include <svtools/templdlg.hxx>
#endif
// -----------------
// for calling basic
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SBXCLASS_HXX
#include <svtools/sbx.hxx>
#endif
#ifndef _SB_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif
#ifndef _SFX_MACROCONF_HXX
#include <sfx2/macrconf.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
// -----------------
#ifndef _DBU_MISCRES_HRC_
#include "dbumiscres.hrc"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
//......................................................................
namespace dbaui
{
//......................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::task;

    //==================================================================
    //= OLinkedDocumentsAccess
    //==================================================================
    //------------------------------------------------------------------
    OLinkedDocumentsAccess::OLinkedDocumentsAccess(Window* _pDialogParent, const Reference< XMultiServiceFactory >& _rxORB, const Reference< XNameAccess >& _rxContainer)
        :m_xORB(_rxORB)
        ,m_xDocumentContainer(_rxContainer)
        ,m_pDialogParent(_pDialogParent)
    {
        OSL_ENSURE(m_xORB.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid service factory!");
        OSL_ENSURE(m_xDocumentContainer.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid document container!");
        OSL_ENSURE(m_pDialogParent, "OLinkedDocumentsAccess::OLinkedDocumentsAccess: really need a dialog parent!");
    }

    //------------------------------------------------------------------
//  Reference< XPropertySet > OLinkedDocumentsAccess::get(const ::rtl::OUString& _rLinkName) SAL_THROW((Exception))
//  {
//      Reference< XPropertySet > xLink;
//      if (m_xDocumentContainer.is())
//          m_xDocumentContainer->getByName(_rLinkName) >>= xLink;
//      return xLink;
//  }

    //------------------------------------------------------------------
    ::rtl::OUString OLinkedDocumentsAccess::getLocation(const ::rtl::OUString& _rLinkName)
    {
        ::rtl::OUString sDocumentLocation;
        try
        {
            if (m_xDocumentContainer.is())
                m_xDocumentContainer->getByName(_rLinkName) >>= sDocumentLocation;
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::getLocation: could not retrieve the link target!");
        }
        return sDocumentLocation;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( OLinkedDocumentsAccess, OnValidateLinkName, String*, _pText )
    {
        OSL_ENSURE(_pText, "OLinkedDocumentsAccess::OnValidateLinkName: invalid text ptr!");
        if (!_pText || 0 == _pText->Len())
            return sal_False;

        try
        {
            if (m_xDocumentContainer->hasByName(*_pText) && (m_sCurrentlyEditing != *_pText))
                return sal_False;
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::OnValidateLinkName: could not ask the collection for the name!");
        }

        return sal_True;
    }

    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::edit(const ::rtl::OUString& _rLinkName, ::rtl::OUString& _rNewName, ::rtl::OUString& _rNewLocation)
    {
        return SUCCESS == implEdit(_rLinkName, _rNewName, _rNewLocation) ? sal_True : sal_False;
    }

    //------------------------------------------------------------------
    OLinkedDocumentsAccess::RESULT OLinkedDocumentsAccess::implEdit(const ::rtl::OUString& _rLinkName, ::rtl::OUString& _rNewName, ::rtl::OUString& _rNewLocation)
    {
        ODocumentLinkDialog aEditLink(m_pDialogParent, sal_False);
        aEditLink.setNameValidator( LINK(this, OLinkedDocumentsAccess, OnValidateLinkName) );

        // retrieve the current objects properties
        String sSelected = _rLinkName;
        String sLocation = getLocation(sSelected);

        {
            OFileNotation aTransformer(sLocation, OFileNotation::N_URL);
            sLocation = aTransformer.get(OFileNotation::N_SYSTEM);
        }

        // set in on the dialog
        m_sCurrentlyEditing = sSelected;
        aEditLink.set( sSelected, sLocation );
        sal_Int32 nResult = aEditLink.Execute();
        m_sCurrentlyEditing = String();

        if (RET_OK != nResult)
            return CANCEL;

        // the new link properties
        String sNewName, sNewLocation;
        aEditLink.get( sNewName, sNewLocation );
        _rNewName = sNewName;
        // the outparam of the location must be a URL, not in system notation
        OFileNotation aTransformer(sNewLocation, OFileNotation::N_SYSTEM);
        _rNewLocation = aTransformer.get(OFileNotation::N_URL);

        sal_Bool bNameChanged = sSelected != sNewName;
        sal_Bool bLocationChanged = sLocation != sNewLocation;
        if (!bNameChanged && !bLocationChanged)
            return CANCEL;

        try
        {
            ::rtl::OUString sOldName = sSelected;

            Reference< XNameContainer > xModifyAccess(m_xDocumentContainer, UNO_QUERY);
            if (xModifyAccess.is())
            {
                if (bNameChanged)
                {   // at least the name changed
                    xModifyAccess->removeByName(sOldName);
                    xModifyAccess->insertByName(sNewName, makeAny(_rNewLocation));
                }
                else
                {   // only the location changed
                    xModifyAccess->replaceByName(sOldName, makeAny(_rNewLocation));
                }
            }
            else
                OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implEdit: no container access! can't rename the object!");

            // flush the container
            Reference< XFlushable > xFlush(m_xDocumentContainer, UNO_QUERY);
            if (xFlush.is())
                xFlush->flush();

            return SUCCESS;
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implEdit: caught an exception while committing the changes!");
            return ERROR;
        }

        OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implEdit: reached the unreachable!");
        return CANCEL;
    }

    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::implFileExists( const ::rtl::OUString& _rURL )
    {
        try
        {
            ::ucb::Content aFile(_rURL, Reference< XCommandEnvironment >());
            if (aFile.isDocument())
                return sal_True;
        }
        catch(Exception&) { }
        return sal_False;
    }

    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::implDrop(const ::rtl::OUString& _rLinkName)
    {
        try
        {
            Reference< XNameContainer > xRemoveAccess(m_xDocumentContainer, UNO_QUERY);
            if (xRemoveAccess.is())
                xRemoveAccess->removeByName(_rLinkName);
            else
                OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implDrop: : missing the XNameContainer interface!");

            // flush the container
            Reference< XFlushable > xFlush(m_xDocumentContainer, UNO_QUERY);
            if (xFlush.is())
                xFlush->flush();
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implDrop: : caught an exception while removing the object!");
        }
    }

    //------------------------------------------------------------------
    OLinkedDocumentsAccess::RESULT OLinkedDocumentsAccess::implOpen(const ::rtl::OUString& _rLinkName, sal_Bool _bReadOnly)
    {
        if (!m_xORB.is() || !m_xDocumentContainer.is())
            return ERROR;

        // get an URL for the document
        ::rtl::OUString sDocumentLocation = getLocation(_rLinkName);

        // check for existence, and show an error message in case the file is not there
        if (!implFileExists(sDocumentLocation))
        {
            OMissingLinkDialog aWhatToDo( m_pDialogParent, sDocumentLocation );
            if (RET_OK != aWhatToDo.Execute())
                return CANCEL;

            if (aWhatToDo.shouldEditLink())
            {
                ::rtl::OUString sNewName, sNewLocation;
                RESULT eResult = implEdit(_rLinkName, sNewName, sNewLocation);
                if (SUCCESS != eResult)
                    return eResult;

                if (implFileExists(sNewLocation))
                    sDocumentLocation = sNewLocation;
                    // proceed with the opening process from here on ....
                else
                    return CANCEL;
            }
            else if (aWhatToDo.shouldDropLink())
            {
                implDrop(_rLinkName);
                return CANCEL;
            }
            else
                return CANCEL;
        }

        Reference< XDispatch > xDispatcher;
        try
        {   // get the desktop object
            Reference< XInterface > xDesktop = m_xORB->createInstance(SERVICE_FRAME_DESKTOP);

            Reference< XComponentLoader > xLoader(xDesktop, UNO_QUERY);
            OSL_ENSURE(xLoader.is(), "OLinkedDocumentsAccess::implOpen: invalid desktop object!");
            if (xLoader.is())
            {
                ::rtl::OUString sTargetFrame = ::rtl::OUString::createFromAscii("_default"); // #104099# OJ

                sal_Int32 nSearchFlags = FrameSearchFlag::CHILDREN | FrameSearchFlag::CREATE;

                Sequence< PropertyValue > aAccessRights(1);
                aAccessRights[0].Name = ::rtl::OUString::createFromAscii("ReadOnly");
                aAccessRights[0].Value = ::cppu::bool2any(_bReadOnly);

                Reference< XComponent > xComp = xLoader->loadComponentFromURL(sDocumentLocation, sTargetFrame, nSearchFlags, aAccessRights);
                return xComp.is() ? SUCCESS : ERROR;
            }
            else
                return ERROR;
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implOpen: caught an exception while retrieving the dispatcher!");
        }

        return ERROR;
    }

    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::addLinkUI()
    {
        // need a container access later on
        Reference< XNameContainer > xContainerAccess(m_xDocumentContainer, UNO_QUERY);
        if (!xContainerAccess.is())
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::addLinkUI: missing a container interface!");
            ErrorBox aError(m_pDialogParent, WB_OK, String(ModuleRes(STR_UNEXPECTED_ERROR)));
            aError.Execute();
            return sal_False;
        }

        ODocumentLinkDialog aEditLink(m_pDialogParent, sal_True);
        aEditLink.setNameValidator( LINK(this, OLinkedDocumentsAccess, OnValidateLinkName) );

        if (RET_OK != aEditLink.Execute())
            return sal_False;

        String sName, sLocation;
        aEditLink.get(sName, sLocation);
        try
        {
            ::rtl::OUString sNormalizedLocation = sLocation;

            OFileNotation aTransformer(sNormalizedLocation, OFileNotation::N_SYSTEM);
            sNormalizedLocation = aTransformer.get(OFileNotation::N_URL);

            xContainerAccess->insertByName(sName, makeAny(sNormalizedLocation));

            // flush the container
            Reference< XFlushable > xFlush(m_xDocumentContainer, UNO_QUERY);
            if (xFlush.is())
                xFlush->flush();
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::addLinkUI: could not insert the new object!");
        }
        return sal_True;
    }

    //------------------------------------------------------------------
    void OLinkedDocumentsAccess::drop(const ::rtl::OUString& _rLinkName)
    {
        // let the user confirm this
        String sQuestion = String(ModuleRes(STR_QUERY_DROP_DOCUMENT_LINK));
        sQuestion.SearchAndReplaceAscii("$name$", _rLinkName);

        QueryBox aAsk(m_pDialogParent, WB_YES_NO | WB_DEF_YES, sQuestion);
        aAsk.SetText(String(ModuleRes(STR_TITLE_CONFIRM_DELETION)));

        if (RET_YES != aAsk.Execute())
            // cancelled
            return;

        implDrop(_rLinkName);
    }

    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::newFormWithPilot(const String& _rDataSourceName, const sal_Int32 _nCommandType,
        const String& _rObjectName, const Reference< XConnection >& _rxConnection)
    {
        SfxApplication* pApp = SFX_APP();
        SbxArray* pParameter            = new SbxArray();
        SbxVariable* pDataSourceName    = new SbxVariable();
        SbxVariable* pContentType       = new SbxVariable();
        SbxVariable* pContent           = new SbxVariable();
        SbxValue* pReturn               = new SbxValue();
        pReturn->AddRef();

        if (0 != _rDataSourceName.Len())
        {
            // add the data source name to the parameter list
            SbxVariable* pArgument = new SbxVariable;
            pArgument->PutString(_rDataSourceName);
            pParameter->Put(pArgument, 1);

            if (_rxConnection.is())
            {
                pParameter->Put(GetSbUnoObject(String::CreateFromAscii("Connection"), makeAny(_rxConnection)), 2);

                if ((-1 != _nCommandType) && _rObjectName.Len())
                {
                    pArgument = new SbxVariable;
                    pArgument->PutLong(_nCommandType);
                    pParameter->Put(pArgument, 3);

                    pArgument = new SbxVariable;
                    pArgument->PutString(_rObjectName);
                    pParameter->Put(pArgument, 4);
                }
            }
        }

        pApp->EnterBasicCall();
        ErrCode aResult = pApp->GetMacroConfig()->Call(NULL, String::CreateFromAscii("FormWizard.FormWizard.MainWithDefault"), pApp->GetBasicManager(), pParameter, pReturn);
        pApp->LeaveBasicCall();

        return ERRCODE_NONE != aResult;
    }
    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::newReportWithPilot(const String& _rDataSourceName, const sal_Int32 _nCommandType,
        const String& _rObjectName, const Reference< XConnection >& _rxConnection)
    {
        try
        {
            ::svx::ODataAccessDescriptor aDesc;
            if ( _rDataSourceName.Len() )
                aDesc[::svx::daDataSource] <<= ::rtl::OUString(_rDataSourceName);
            if ( _nCommandType != -1 )
                aDesc[::svx::daCommandType] <<= _nCommandType;
            if ( _rObjectName.Len() )
                aDesc[::svx::daCommand] <<= ::rtl::OUString(_rObjectName);
            if ( _rxConnection.is() )
                aDesc[::svx::daConnection] <<= _rxConnection;

            Reference< XJobExecutor > xReportWizard(m_xORB->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.wizards.report.CallReportWizard")),aDesc.createAnySequence()),UNO_QUERY);
            if ( xReportWizard.is() )
                xReportWizard->trigger(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("start")));

        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newReport: caught an exception while loading the object!");
        }
        return sal_True;
    }
    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::newForm(sal_Int32 _nNewFormId)
    {
        // determine the URL to use for the new document
        ::rtl::OUString sDocumentURL;
        switch (_nNewFormId)
        {
            case ID_FORM_NEW_TEXT:
                sDocumentURL = ::rtl::OUString::createFromAscii("private:factory/swriter");
                break;

            case ID_FORM_NEW_CALC:
                sDocumentURL = ::rtl::OUString::createFromAscii("private:factory/scalc");
                break;

            case ID_FORM_NEW_IMPRESS:
                sDocumentURL = ::rtl::OUString::createFromAscii("private:factory/simpress");
                break;

            case ID_FORM_NEW_PILOT:
                OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newForm: pleas use newFormWithPilot!");
                return sal_False;

            case ID_FORM_NEW_TEMPLATE:
            {
                SvtDocumentTemplateDialog aDialog( m_pDialogParent, SvtDocumentTemplateDialog::SelectOnly() );
                if ( ( RET_OK == aDialog.Execute() ) && aDialog.IsFileSelected() )
                    sDocumentURL = aDialog.GetSelectedFileURL( );
                else
                    return sal_False;
            }
            break;
        }

        if (0 == sDocumentURL.getLength())
            return sal_False;

        // load the document as template
        Reference< XComponent > xNewDocument;
        try
        {   // get the desktop object
            Reference< XInterface > xDesktop = m_xORB->createInstance(SERVICE_FRAME_DESKTOP);

            Reference< XComponentLoader > xLoader(xDesktop, UNO_QUERY);
            OSL_ENSURE(xLoader.is(), "OLinkedDocumentsAccess::newForm: invalid desktop object!");
            if (xLoader.is())
            {
                ::rtl::OUString sTargetFrame = ::rtl::OUString::createFromAscii("_default"); // #104099# OJ

                sal_Int32 nSearchFlags = FrameSearchFlag::CHILDREN | FrameSearchFlag::CREATE;

                Sequence< PropertyValue > aAccessRights(1);
                aAccessRights[0].Name = ::rtl::OUString::createFromAscii("AsTemplate");
                aAccessRights[0].Value = ::cppu::bool2any(sal_True);

                xNewDocument = xLoader->loadComponentFromURL(sDocumentURL, sTargetFrame, nSearchFlags, aAccessRights);
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newForm: caught an exception while loading the object!");
        }

        if (!xNewDocument.is())
        {
            // TODO: error message
            return sal_False;
        }

        Reference< XModel > xDocModel(xNewDocument, UNO_QUERY);
        Reference< XNameContainer > xContainerAccess(m_xDocumentContainer, UNO_QUERY);
        if (xDocModel.is() && xContainerAccess.is())
        {
            ODocumentAutoLinker* pAutoDocumentLink = new ODocumentAutoLinker(xDocModel, xContainerAccess);
            Reference< XInterface > xEnsureDelete(*pAutoDocumentLink);
        }
        else
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newForm: invalid doc model or invalid link container (unable to automatically register the file)!");

        return sal_True;
    }

    //------------------------------------------------------------------
    sal_Bool OLinkedDocumentsAccess::open(const ::rtl::OUString& _rLinkName, sal_Bool _bReadOnly)
    {
        RESULT eResult = implOpen(_rLinkName, _bReadOnly);
        switch (eResult)
        {
            case ERROR:
            {
                String sLocation = getLocation(_rLinkName);
                OFileNotation aTransformer(sLocation, OFileNotation::N_URL);
                sLocation = aTransformer.get(OFileNotation::N_SYSTEM);

                String sMessage = String(ModuleRes(STR_COULDNOTOPEN_LINKEDDOC));
                sMessage.SearchAndReplaceAscii("$file$", sLocation);
                ErrorBox aError(m_pDialogParent, WB_OK, sMessage);
                aError.Execute();

                return sal_False;
            }

            case CANCEL:
                return sal_False;

            case SUCCESS:
                return sal_True;
        }

        OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::open: reached the unreachable!");
        return sal_False;
    }
//......................................................................
}   // namespace dbaui
//......................................................................

