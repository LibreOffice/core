/*************************************************************************
 *
 *  $RCSfile: linkeddocuments.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:10:53 $
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
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
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
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
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
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _SFXNEW_HXX
#include <sfx2/new.hxx>
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
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMECONTAINER_HPP_
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
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
    using namespace ::svt;

    namespace
    {
        Sequence< sal_Int8 > lcl_GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                    sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                    sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
        {
            Sequence< sal_Int8 > aResult( 16 );
            aResult[0] = static_cast<sal_Int8>(n1 >> 24);
            aResult[1] = static_cast<sal_Int8>(( n1 << 8 ) >> 24);
            aResult[2] = static_cast<sal_Int8>(( n1 << 16 ) >> 24);
            aResult[3] = static_cast<sal_Int8>(( n1 << 24 ) >> 24);
            aResult[4] = n2 >> 8;
            aResult[5] = ( n2 << 8 ) >> 8;
            aResult[6] = n3 >> 8;
            aResult[7] = ( n3 << 8 ) >> 8;
            aResult[8] = b8;
            aResult[9] = b9;
            aResult[10] = b10;
            aResult[11] = b11;
            aResult[12] = b12;
            aResult[13] = b13;
            aResult[14] = b14;
            aResult[15] = b15;

            return aResult;
        }
    }


    //==================================================================
    //= OLinkedDocumentsAccess
    //==================================================================
    //------------------------------------------------------------------
    OLinkedDocumentsAccess::OLinkedDocumentsAccess(Window* _pDialogParent
                                                    , const Reference< XMultiServiceFactory >& _rxORB
                                                    , const Reference< XNameAccess >& _rxContainer
                                                    ,const Reference< XConnection>& _xConnection
                                                    )
        :m_xORB(_rxORB)
        ,m_xDocumentContainer(_rxContainer)
        ,m_pDialogParent(_pDialogParent)
        ,m_xConnection(_xConnection)
    {
        OSL_ENSURE(m_xORB.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid service factory!");
        OSL_ENSURE(m_xDocumentContainer.is(), "OLinkedDocumentsAccess::OLinkedDocumentsAccess: invalid document container!");
        OSL_ENSURE(m_pDialogParent, "OLinkedDocumentsAccess::OLinkedDocumentsAccess: really need a dialog parent!");
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
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::implDrop: : caught an exception while removing the object!");
        }
    }

    //------------------------------------------------------------------
    Reference< XComponent> OLinkedDocumentsAccess::implOpen(const ::rtl::OUString& _rLinkName,Reference< XComponent >& _xDefinition, sal_Bool _bReadOnly)
    {
        Reference< XComponent> xRet;
        Reference< XComponentLoader > xComponentLoader(m_xDocumentContainer,UNO_QUERY);
        if ( !xComponentLoader.is() )
            return xRet;

        Sequence< PropertyValue > aArguments(2);

        aArguments[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OpenMode"));
        aArguments[0].Value <<= (_bReadOnly ? ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open")) : ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("openDesign")) );

        aArguments[1].Name = PROPERTY_ACTIVECONNECTION;
        aArguments[1].Value <<= m_xConnection;
        try
        {
            Reference<XHierarchicalNameContainer> xHier(m_xDocumentContainer,UNO_QUERY);
            if ( xHier.is() && xHier->hasByHierarchicalName(_rLinkName) )
            {
                _xDefinition.set(xHier->getByHierarchicalName(_rLinkName),UNO_QUERY);
            }
            xRet = xComponentLoader->loadComponentFromURL(_rLinkName,::rtl::OUString(),0,aArguments);
        }
        catch(Exception& )
        {
        }

        return xRet;
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
            aDesc.setDataSource(::rtl::OUString(_rDataSourceName));
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
    sal_Bool OLinkedDocumentsAccess::newQueryWithPilot(const String& _rDataSourceName, const sal_Int32 _nCommandType,
        const String& _rObjectName, const Reference< XConnection >& _rxConnection)
    {
        try
        {
            ::svx::ODataAccessDescriptor aDesc;
            if ( _rDataSourceName.Len() )
                aDesc.setDataSource( ::rtl::OUString(_rDataSourceName) );
            if ( _nCommandType != -1 )
                aDesc[::svx::daCommandType] <<= _nCommandType;
            if ( _rObjectName.Len() )
                aDesc[::svx::daCommand] <<= ::rtl::OUString(_rObjectName);
            if ( _rxConnection.is() )
                aDesc[::svx::daConnection] <<= _rxConnection;

            Reference< XJobExecutor > xQueryWizard(m_xORB->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.wizards.query.CallQueryWizard")),aDesc.createAnySequence()),UNO_QUERY);
            if ( xQueryWizard.is() )
                xQueryWizard->trigger(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("start")));

        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newQueryWithPilot: caught an exception while loading the object!");
        }
        return sal_True;
    }
    //------------------------------------------------------------------
    Reference< XComponent > OLinkedDocumentsAccess::newForm(sal_Int32 _nNewFormId,Reference< XComponent >& _xDefinition)
    {
        // determine the URL to use for the new document
        Sequence<sal_Int8> aClassId;
        switch (_nNewFormId)
        {
            case ID_FORM_NEW_TEXT:
                aClassId = lcl_GetSequenceClassID(SO3_SW_CLASSID);
                break;

            case ID_FORM_NEW_CALC:
                aClassId = lcl_GetSequenceClassID(SO3_SC_CLASSID);
                break;

            case ID_FORM_NEW_IMPRESS:
                aClassId = lcl_GetSequenceClassID(SO3_SIMPRESS_CLASSID);
                break;

            case ID_FORM_NEW_PILOT:
            default:
                OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newForm: pleas use newFormWithPilot!");
                return Reference< XComponent >();

        }
        // load the document as template
        Reference< XComponent > xNewDocument;
        try
        {   // get the desktop object

            Reference<XMultiServiceFactory> xORB(m_xDocumentContainer,UNO_QUERY);
            if ( xORB.is() )
            {
                Sequence< Any > aArguments(2);

                PropertyValue aValue;

                aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ClassID"));
                aValue.Value <<= aClassId;
                aArguments[0] <<= aValue;

                aValue.Name = PROPERTY_ACTIVECONNECTION;
                aValue.Value <<= m_xConnection;
                aArguments[1] <<= aValue;

                Reference<XCommandProcessor> xContent(xORB->createInstanceWithArguments(SERVICE_SDB_DOCUMENTDEFINITION,aArguments),UNO_QUERY);
                if ( xContent.is() )
                {
                    _xDefinition.set(xContent,UNO_QUERY);
                    Command aCommand;
                    aCommand.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("openDesign"));
                    OpenCommandArgument2 aOpenCommand;
                    aOpenCommand.Mode = OpenMode::DOCUMENT;
                    aCommand.Argument <<= aOpenCommand;
                    xNewDocument.set(xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >()),UNO_QUERY);
                }
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "OLinkedDocumentsAccess::newForm: caught an exception while loading the object!");
        }

        return xNewDocument;
    }

    //------------------------------------------------------------------
    Reference< XComponent > OLinkedDocumentsAccess::open(const ::rtl::OUString& _rLinkName,Reference< XComponent >& _xDefinition, sal_Bool _bReadOnly)
    {
        Reference< XComponent > xRet = implOpen(_rLinkName,_xDefinition, _bReadOnly);
        if ( !xRet.is() )
        {
            String sMessage = String(ModuleRes(STR_COULDNOTOPEN_LINKEDDOC));
            sMessage.SearchAndReplaceAscii("$file$",_rLinkName);
            ErrorBox aError(m_pDialogParent, WB_OK, sMessage);
            aError.Execute();
        }
        return xRet;
    }
//......................................................................
}   // namespace dbaui
//......................................................................

