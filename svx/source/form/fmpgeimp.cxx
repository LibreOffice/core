/*************************************************************************
 *
 *  $RCSfile: fmpgeimp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:17 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define _EXTERN_FORMLAYER_


#ifndef _SVXERR_HXX
#include "svxerr.hxx"
#endif

#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif

#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

//  #include "fmstream.hxx"
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif
#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _UTL_UNO3_HXX_
#include <unotools/uno3.hxx>
#endif
#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _UTL_UNO3_DB_TOOLS_HXX_
#include <unotools/dbtools.hxx>
#endif

#define ANY_TO_IFACE(any)   (*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > *)any.getValue())

//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage)
               :pPage(_pPage)
{
    Init();
}

//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage, const FmFormPageImpl& rImpl)
               :pPage(_pPage)
{
    Init();

    // copy it by streaming
    // creating a pipe
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >  xOutPipe(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.Pipe")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  xInPipe(xOutPipe, ::com::sun::star::uno::UNO_QUERY);

    // creating the mark streams
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  xMarkIn(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink >  xMarkSink(xMarkIn, ::com::sun::star::uno::UNO_QUERY);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >  xMarkOut(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  xMarkSource(xMarkOut, ::com::sun::star::uno::UNO_QUERY);

    // connect pipe and sink
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink >  xSink(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), ::com::sun::star::uno::UNO_QUERY);

    // connect pipe and source
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  xSource(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), ::com::sun::star::uno::UNO_QUERY);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >  xOutStrm(xSource, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >  xInStrm(xSink, ::com::sun::star::uno::UNO_QUERY);

    if (xMarkSink.is() && xMarkSource.is() && xSink.is() && xSource.is() && xOutStrm.is() && xInStrm.is())
    {
        xMarkSink->setInputStream(xInPipe);
        xMarkSource->setOutputStream(xOutPipe);
        xSink->setInputStream(xMarkIn);
        xSource->setOutputStream(xMarkOut);

        // write the objects to source
        rImpl.write(xOutStrm);
        xOutStrm->closeOutput();

        // read them
        read(xInStrm);
        xInStrm->closeInput();
    }
    //  what to do else ?
}

//------------------------------------------------------------------------------
void FmFormPageImpl::Init()
{
    if (pPage)
    {
        FmFormModel* pDrawModel = (FmFormModel*)pPage->GetModel();
        SfxObjectShell* pObjShell = pDrawModel->GetObjectShell();
        if( pObjShell )
            xModel = pObjShell->GetModel();

        // get a unique page id from the model
        m_sPageId = pDrawModel->GetUniquePageId();
    }

    xForms = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > (
        ::utl::getProcessServiceFactory()->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.form.FormsCollection")), ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xForms.is(), "FmFormPageImpl::Init : could not create a forms collection !");

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xAsChild(xForms, ::com::sun::star::uno::UNO_QUERY);
    if (xAsChild.is())
        xAsChild->setParent( xModel );
}

//------------------------------------------------------------------------------
FmFormPageImpl::~FmFormPageImpl()
{
    xCurrentForm = NULL;

    ::utl::disposeComponent(xForms);
}

//------------------------------------------------------------------------------
void FmFormPageImpl::setCurForm(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm)
{
    xCurrentForm = xForm;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::getDefaultForm()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm;

    // wenn noch kein TargetForm gefunden, dann aktuelle oder Default
    if (!xCurrentForm.is())
    {
        if (xForms->hasElements())
        {
            // suche die Standardform
            ::rtl::OUString ustrStdFormName = ::rtl::OUString(SVX_RES(RID_STR_STDFORMNAME));

            if (xForms->hasByName(ustrStdFormName))
            {
                try
                {
                    xForm = *(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > *)xForms->getByName(ustrStdFormName).getValue();
                }
                catch(...)
                {
                }

            }

            // gibt es denn ueberhaupt eine
            if (!xForm.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xGetFirst(xForms, ::com::sun::star::uno::UNO_QUERY);
                DBG_ASSERT(xGetFirst.is(), "FmFormPageImpl::getDefaultForm : no IndexAccess on my form container !");
                    // wenn das anspringt, muesste man sich die Namen des NameContainers geben lassen und dann das Objekt fuer den
                    // ersten Namen erfragen ... aber normalerweise sollte die FOrms-Sammlung auch einen IndexAccess haben
                xForm = *(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > *)xGetFirst->getByIndex(0).getValue();
            }
        }
    }
    else
        xForm = xCurrentForm;

    // keine gefunden dann standard erzeugen
    if (!xForm.is())
    {

        SdrModel* pModel = pPage->GetModel();
        XubString aStr(SVX_RES(RID_STR_FORM));
        XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
        aUndoStr.SearchAndReplace('#', aStr);
        pModel->BegUndo(aUndoStr);

        xForm = ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >(::utl::getProcessServiceFactory()->createInstance(FM_SUN_COMPONENT_FORM), ::com::sun::star::uno::UNO_QUERY);
        // a form should always have the command type table as default
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(xForm, ::com::sun::star::uno::UNO_QUERY);
        try
        {
            xSet->setPropertyValue(FM_PROP_COMMANDTYPE, ::com::sun::star::uno::makeAny(sal_Int32(::com::sun::star::sdb::CommandType::TABLE)));
        }
        catch(...)
        {
        }

        ::rtl::OUString aName = ::rtl::OUString(SVX_RES(RID_STR_STDFORMNAME));
        xSet->setPropertyValue(FM_PROP_NAME, ::com::sun::star::uno::makeAny(aName));


        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xForms, ::com::sun::star::uno::UNO_QUERY);
        pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                   FmUndoContainerAction::Inserted,
                                                   xContainer,
                                                   xForm,
                                                   xContainer->getCount()));
        xForms->insertByName(aName, ::com::sun::star::uno::makeAny(xForm));
        xCurrentForm = xForm;
        pModel->EndUndo();
    }
    return xForm;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::SetDefaults(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & rContent,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseAccess > & rDatabase,
                                     const ::rtl::OUString& rDBTitle,
                                     const ::rtl::OUString& rCursorSource,
                                     sal_Int32 nCommandType)
{
    // Ist das Control bereits einer ::com::sun::star::form zugeordnet
    if (!rContent.is() || rContent->getParent().is())
        return NULL;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet;

    // Wenn Datenbank und CursorSource gesetzt sind, dann wird
    // die ::com::sun::star::form anhand dieser Kriterien gesucht, ansonsten nur aktuelle
    // und die StandardForm
    if (rDatabase.is() && rCursorSource.len())
    {
        // erst in der aktuellen form suchen
        xForm = FindForm(xCurrentForm, rDatabase, rCursorSource, nCommandType);

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xFormsByIndex(xForms, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xFormsByIndex.is(), "FmFormPageImpl::SetDefaults : no index access for my forms collection !");
        sal_Int32 nCount = xFormsByIndex->getCount();
        for (sal_Int32 i = 0; !xForm.is() && i < nCount; i++)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xToSearch(*(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > *)xFormsByIndex->getByIndex(i).getValue());
            xForm = FindForm(xToSearch, rDatabase, rCursorSource, nCommandType);
        }

        // wenn keine ::com::sun::star::form gefunden, dann eine neue erzeugen
        if (!xForm.is())
        {
            SdrModel* pModel = pPage->GetModel();
            XubString aStr(SVX_RES(RID_STR_FORM));
            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
            aUndoStr.SearchAndReplace('#', aStr);
            pModel->BegUndo(aUndoStr);
            xForm = ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >(::utl::getProcessServiceFactory()->createInstance(FM_SUN_COMPONENT_FORM), ::com::sun::star::uno::UNO_QUERY);
            // a form should always have the command type table as default
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(xForm, ::com::sun::star::uno::UNO_QUERY);
            try { xSet->setPropertyValue(FM_PROP_COMMANDTYPE, ::com::sun::star::uno::makeAny(sal_Int32(::com::sun::star::sdb::CommandType::TABLE))); }
            catch(...) { }

            if (rDBTitle.len())
                xSet->setPropertyValue(FM_PROP_DATASOURCE,::com::sun::star::uno::makeAny(rDBTitle));
            else
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xDatabaseProps(rDatabase, ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Any aDatabaseUrl = xDatabaseProps->getPropertyValue(FM_PROP_URL);
                xSet->setPropertyValue(FM_PROP_DATASOURCE, aDatabaseUrl);
            }

            xSet->setPropertyValue(FM_PROP_COMMAND,::com::sun::star::uno::makeAny(rCursorSource));
            xSet->setPropertyValue(FM_PROP_COMMANDTYPE, ::com::sun::star::uno::makeAny(nCommandType));

            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  xNamedSet(xForms, ::com::sun::star::uno::UNO_QUERY);
            ::rtl::OUString aName;

            if ((::com::sun::star::sdb::CommandType::TABLE == nCommandType) || (::com::sun::star::sdb::CommandType::QUERY == nCommandType))
            {
                // Namen der ::com::sun::star::form ueber den Titel der CursorSource setzen
                aName = getUniqueName(rCursorSource, xNamedSet);
            }
            else
                // ansonsten StandardformName verwenden
                aName = getUniqueName(::rtl::OUString(SVX_RES(RID_STR_STDFORMNAME)), xNamedSet);

            xSet->setPropertyValue(FM_PROP_NAME, ::com::sun::star::uno::makeAny(aName));

            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xForms, ::com::sun::star::uno::UNO_QUERY);
            pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                     FmUndoContainerAction::Inserted,
                                                     xContainer,
                                                     xForm,
                                                     xContainer->getCount()));


            xForms->insertByName(aName, ::com::sun::star::uno::makeAny(xForm));
            pModel->EndUndo();
        }
        xCurrentForm = xForm;
    }

    xForm = getDefaultForm();
    // eindeutigen Namen fuer die Componente setzen
    setUniqueName(rContent, xForm);
    return xForm;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::FindForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & rForm,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseAccess > & rDatabase,
                                  const ::rtl::OUString& rCursorSource,
                                  sal_Int32 nCommandType)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xResultForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xDBForm(rForm, ::com::sun::star::uno::UNO_QUERY);
    if (!xDBForm.is())
        return NULL;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xConnAsChild;
    try
    {
        xConnAsChild = ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > (::utl::calcConnection(xDBForm,::utl::getProcessServiceFactory()), ::com::sun::star::uno::UNO_QUERY);
    }
    catch(...)
    {
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseAccess >  xDB;
    if (xConnAsChild.is())
        xDB = ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseAccess > (xConnAsChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
    if (xDB.is() && xDB == rDatabase)
    {
        // jetzt noch ueberpruefen ob CursorSource und Type uebereinstimmen
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(rForm, ::com::sun::star::uno::UNO_QUERY);
        ::rtl::OUString aCursorSource = ::utl::getString(xSet->getPropertyValue(FM_PROP_COMMAND));
        sal_Int32 nType = ::utl::getINT32(xSet->getPropertyValue(FM_PROP_COMMANDTYPE));
        if (!aCursorSource.len() || ((nType == nCommandType) && (aCursorSource == rCursorSource))) // found the form
        {
            xResultForm = rForm;
            // Ist noch keine Datenquelle gesetzt, wird dieses hier nachgeholt
            if (!aCursorSource.len())
            {
                xSet->setPropertyValue(FM_PROP_COMMAND, ::com::sun::star::uno::makeAny(rCursorSource));
                xSet->setPropertyValue(FM_PROP_COMMANDTYPE, ::com::sun::star::uno::makeAny((sal_Int32)nCommandType));
            }
        }
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xComponents(rForm, ::com::sun::star::uno::UNO_QUERY);
    sal_Int32 nCount = xComponents->getCount();
    for (sal_Int32 i = 0; !xResultForm.is() && i < nCount; ++i)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xSearchForm(ANY_TO_IFACE(xComponents->getByIndex(i)), ::com::sun::star::uno::UNO_QUERY);
        // jetzt innerhalb der ::com::sun::star::form weitersuchen
        if (xSearchForm.is())
            xResultForm = FindForm(xSearchForm, rDatabase, rCursorSource, nCommandType);
    }
    return xResultForm;
}

//------------------------------------------------------------------------------
::rtl::OUString FmFormPageImpl::setUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xFormComponent, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xControls)
{
    ::rtl::OUString sName;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(xFormComponent, ::com::sun::star::uno::UNO_QUERY);
    if (xSet.is())
    {
        ::com::sun::star::uno::Any aValue = xSet->getPropertyValue(FM_PROP_NAME);
        sName = ::utl::getString(aValue);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  xNameAcc(xControls, ::com::sun::star::uno::UNO_QUERY);

        if (!sName.len() || xNameAcc->hasByName(sName))
        {
            // setzen eines default Namens ueber die ClassId
            ::com::sun::star::uno::Any aValue = xSet->getPropertyValue(FM_PROP_CLASSID);
            sal_Int16 nClassId(::com::sun::star::form::FormComponentType::CONTROL);
            aValue >>= nClassId;

            ::rtl::OUString sDefaultName = getDefaultName(nClassId, xControls);
            // bei Radiobuttons, die einen Namen haben, diesen nicht ueberschreiben!
            if (!sName.len() || nClassId != ::com::sun::star::form::FormComponentType::RADIOBUTTON)
            {
                xSet->setPropertyValue(FM_PROP_NAME, ::com::sun::star::uno::makeAny(sDefaultName));
            }

            //////////////////////////////////////////////////////////////
            // Labels anpassen
            UniString aLabel = sDefaultName;
            sal_uInt16 nResId = 0;

            switch (nClassId)
            {
                case ::com::sun::star::form::FormComponentType::COMMANDBUTTON:  nResId = RID_STR_PROPTITLE_PUSHBUTTON;      break;
                case ::com::sun::star::form::FormComponentType::RADIOBUTTON:    nResId = RID_STR_PROPTITLE_RADIOBUTTON;     break;
                case ::com::sun::star::form::FormComponentType::CHECKBOX:       nResId = RID_STR_PROPTITLE_CHECKBOX;        break;
                case ::com::sun::star::form::FormComponentType::GROUPBOX:       nResId = RID_STR_PROPTITLE_GROUPBOX;        break;
                case ::com::sun::star::form::FormComponentType::FIXEDTEXT:      nResId = RID_STR_PROPTITLE_FIXEDTEXT;       break;
            }

            if (nResId)
            {
                aValue = xSet->getPropertyValue(FM_PROP_LABEL);
                ::rtl::OUString aText;
                aValue >>= aText;
                if (!aText.len())
                {
                    aLabel.SearchAndReplace( getDefaultName(nClassId), ::rtl::OUString(SVX_RES(nResId)) );
                    xSet->setPropertyValue( FM_PROP_LABEL, ::com::sun::star::uno::makeAny(::rtl::OUString(aLabel)) );
                }
            }

            sName = sDefaultName;
        }
    }
    return sName;
}


UniString FmFormPageImpl::getDefaultName(sal_Int16 nClassId)
{
    sal_uInt16 nResId;

    switch (nClassId)
    {
        case ::com::sun::star::form::FormComponentType::TEXTFIELD:      nResId = RID_STR_EDIT_CLASSNAME;        break;
        case ::com::sun::star::form::FormComponentType::COMMANDBUTTON:  nResId = RID_STR_BUTTON_CLASSNAME;      break;
        case ::com::sun::star::form::FormComponentType::RADIOBUTTON:    nResId = RID_STR_RADIOBUTTON_CLASSNAME; break;
        case ::com::sun::star::form::FormComponentType::CHECKBOX:       nResId = RID_STR_CHECKBOX_CLASSNAME;    break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:        nResId = RID_STR_LISTBOX_CLASSNAME;     break;
        case ::com::sun::star::form::FormComponentType::COMBOBOX:       nResId = RID_STR_COMBOBOX_CLASSNAME;    break;
        case ::com::sun::star::form::FormComponentType::GROUPBOX:       nResId = RID_STR_GROUPBOX_CLASSNAME;    break;
        case ::com::sun::star::form::FormComponentType::IMAGEBUTTON:    nResId = RID_STR_IMAGE_CLASSNAME;       break;
        case ::com::sun::star::form::FormComponentType::FIXEDTEXT:      nResId = RID_STR_FIXEDTEXT_CLASSNAME;   break;
        case ::com::sun::star::form::FormComponentType::GRIDCONTROL:    nResId = RID_STR_GRID_CLASSNAME;        break;
        case ::com::sun::star::form::FormComponentType::FILECONTROL:    nResId = RID_STR_FILECONTROL_CLASSNAME; break;

        case ::com::sun::star::form::FormComponentType::DATEFIELD:      nResId = RID_STR_DATEFIELD_CLASSNAME;   break;
        case ::com::sun::star::form::FormComponentType::TIMEFIELD:      nResId = RID_STR_TIMEFIELD_CLASSNAME;   break;
        case ::com::sun::star::form::FormComponentType::NUMERICFIELD:   nResId = RID_STR_NUMERICFIELD_CLASSNAME;    break;
        case ::com::sun::star::form::FormComponentType::CURRENCYFIELD:  nResId = RID_STR_CURRENCYFIELD_CLASSNAME;   break;
        case ::com::sun::star::form::FormComponentType::PATTERNFIELD:   nResId = RID_STR_PATTERNFIELD_CLASSNAME;    break;
        case ::com::sun::star::form::FormComponentType::IMAGECONTROL:   nResId = RID_STR_IMAGECONTROL_CLASSNAME;    break;
        case ::com::sun::star::form::FormComponentType::HIDDENCONTROL:  nResId = RID_STR_HIDDEN_CLASSNAME;      break;

        case ::com::sun::star::form::FormComponentType::CONTROL:
        default:
            nResId = RID_STR_CONTROL_CLASSNAME;     break;
    }

    return SVX_RES(nResId);
}

//------------------------------------------------------------------------------
::rtl::OUString FmFormPageImpl::getDefaultName(sal_Int16 nClassId, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xControls) const
{
    ::rtl::OUString aClassName=getDefaultName(nClassId);

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  xNamedSet(xControls, ::com::sun::star::uno::UNO_QUERY);
    return getUniqueName(aClassName, xNamedSet);
}

//------------------------------------------------------------------
::rtl::OUString FmFormPageImpl::getUniqueName(const ::rtl::OUString& rName, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNamedSet) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xIndexSet(xNamedSet, ::com::sun::star::uno::UNO_QUERY);
    ::rtl::OUString sName;

    if ( !xIndexSet.is() )
        return sName;

    sal_Int32 n = 0;
    ::rtl::OUString sClassName = rName;
    do
    {
        sName = sClassName + ::rtl::OUString::valueOf(++n);
    }   while (xNamedSet->hasByName(sName));
    return sName;
}

//------------------------------------------------------------------------------
void FmFormPageImpl::WriteData(SvStream& rOut) const
{
    // anlegen eines output streams fuer UNO
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  xSource(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >  xMarkOut(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSource >  xMarkSource(xMarkOut, ::com::sun::star::uno::UNO_QUERY);

    if (xSource.is())
    {
        xMarkSource->setOutputStream(new ::utl::OOutputStreamWrapper(rOut));
        xSource->setOutputStream(xMarkOut);

        ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >  xOutStrm(xSource, ::com::sun::star::uno::UNO_QUERY);
        try
        {
            write(xOutStrm);
        }
        catch(...)
        {
            rOut.SetError( ERRCODE_CLASS_WRITE | ERRCODE_SVX_FORMS_READWRITEFAILED | ERRCODE_WARNING_MASK );
        }

        xOutStrm->closeOutput();
    }
    else
    {
        // let's do a hack : in former versions we didn't use UNO-stream but SvStreams, so there was
        // no possibility that we couldn't write because of the lack of stream objects. Now, with UNO, this may happen
        // (as it happended here). But the read-methods of the older versions don't expect that (there is no flag in
        // the fileformat as it never was neccessary). To allow the old versions the read of the documents, we fake the
        // write : the first token usually written by our impl is an sal_Int32 giving the number of forms within the forms
        // collection. If this number is 0, no further reading will occur.
        sal_Int32 nDummLength = 0; rOut << nDummLength;     // this means 'no forms'
        nDummLength = 0; rOut << nDummLength;           // this means 'no form controls"
        // To tell the user that something went wrong we set a warning on the stream.
        rOut.SetError( ERRCODE_CLASS_WRITE | ERRCODE_SVX_FORMS_NOIOSERVICES | ERRCODE_WARNING_MASK );
    }
}

//------------------------------------------------------------------------------
void FmFormPageImpl::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    // Abholen des InputStreams ueber uno
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink >  xSink(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), ::com::sun::star::uno::UNO_QUERY);

    // creating the mark streams
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  xMarkIn(::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XActiveDataSink >  xMarkSink(xMarkIn, ::com::sun::star::uno::UNO_QUERY);

    if (xSink.is())
    {
        ::utl::OInputStreamWrapper* pUnoStream = new ::utl::OInputStreamWrapper(rIn);
        xMarkSink->setInputStream(::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > (pUnoStream));
        xSink->setInputStream(xMarkIn);

        // freigeben aller forms
        xCurrentForm = NULL;

        if (xForms->hasElements())
        {
            ::utl::disposeComponent(xForms);
            Init();
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >  xInStrm(xSink,::com::sun::star::uno::UNO_QUERY);
        try
        {
            read(xInStrm);
        }
        catch(...)
        {
            rIn.SetError( ERRCODE_CLASS_READ | ERRCODE_SVX_FORMS_READWRITEFAILED | ERRCODE_WARNING_MASK );
        }


        xInStrm->closeInput();
    }
    else
        rIn.SetError( ERRCODE_CLASS_READ | ERRCODE_SVX_FORMS_NOIOSERVICES | ERRCODE_WARNING_MASK );
}

//------------------------------------------------------------------------------
void FmFormPageImpl::write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream > & xOutStrm) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMarkStrm(xOutStrm, ::com::sun::star::uno::UNO_QUERY);
    if (!xMarkStrm.is())
        return; // exception

    //  sortieren der objectlist nach der Reihenfolge
    FmObjectList aList;
    fillList(aList, *pPage, sal_True);

    // schreiben aller forms
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xAsPersist(xForms, ::com::sun::star::uno::UNO_QUERY);
    if (xAsPersist.is())
        xAsPersist->write(xOutStrm);
        // don't use the writeObject of the stream, as this wouldn't be compatible with older documents

    // objectliste einfuegen
    sal_Int32 nLength = aList.Count();

    // schreiben der laenge
    xOutStrm->writeLong(nLength);

    for (sal_Int32 i = 0; i < nLength; i++)
    {
        // schreiben des Objects mit Marke
        // Marke um an den Anfang zu springen
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xObj(aList.GetObject(i)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xObj.is())
        {
            xOutStrm->writeObject(xObj);
        }
        else
            ;// exception
    }
}

//------------------------------------------------------------------------------
void FmFormPageImpl::read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream > & xInStrm)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XMarkableStream >  xMarkStrm(xInStrm, ::com::sun::star::uno::UNO_QUERY);
    if (!xMarkStrm.is())
        return; // exception

    //  sortieren der objectlist nach der Reihenfolge
    FmObjectList aList;
    fillList(aList, *pPage, sal_False);

    // lesen aller forms
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject >  xAsPersist(xForms, ::com::sun::star::uno::UNO_QUERY);
    if (xAsPersist.is())
        xAsPersist->read(xInStrm);
        // don't use the readObject of the stream, as this wouldn't be compatible with older documents

    // Zuordnung der Formobjekte zu den FormComponents
    sal_Int32 nLength = xInStrm->readLong();
    DBG_ASSERT(nLength == (sal_Int32) aList.Count(), "Fehler beim Lesen der UnoModels");
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >  xRef(xInStrm->readObject(), ::com::sun::star::uno::UNO_QUERY);
        if (i < (sal_Int32)aList.Count())
            aList.GetObject(i)->SetUnoControlModel(xRef);
    }
}

//------------------------------------------------------------------------------
void FmFormPageImpl::fillList(FmObjectList& rList, const SdrObjList& rObjList, sal_Bool bConnected) const
{
    SdrObjListIter aIter(rObjList);
    while (aIter.IsMore())
    {
        SdrObject* pObj = aIter.Next();
        if (pObj && pObj->GetObjInventor() == FmFormInventor)
        {
            FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
            DBG_ASSERT(!bConnected || pFormObj->GetUnoControlModel().is(), "Controlshape ohne Control");
            if (!bConnected || pFormObj->GetUnoControlModel().is())
                rList.Insert(pFormObj, LIST_APPEND);

        }
    }
}


