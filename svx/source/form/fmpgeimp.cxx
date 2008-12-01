/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmpgeimp.cxx,v $
 * $Revision: 1.38 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "svxerr.hxx"
#include "fmpgeimp.hxx"
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <sfx2/objsh.hxx>
#include "fmundo.hxx"
#include "fmtools.hxx"
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#include <svx/fmglob.hxx>
#include "fmservs.hxx"

//  #include "fmstream.hxx"
#include "fmobj.hxx"
#include <svx/fmpage.hxx>
#include <svx/fmmodel.hxx>
#include <tools/resid.hxx>
#include <tools/diagnose_ex.h>
#include "svditer.hxx"

#include "fmresids.hrc"
#include <tools/shl.hxx>
#include <vcl/stdtext.hxx>
#include <svx/dialmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <unotools/streamwrap.hxx>
#include "dbtoolsclient.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::svxform;

DBG_NAME(FmFormPageImpl)
//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage)
               :pPage(_pPage)
               ,m_bFirstActivation( sal_True )
               ,m_bAttemptedFormCreation( false )
{
    DBG_CTOR(FmFormPageImpl,NULL);
}

//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage, const FmFormPageImpl& rImpl)
               :pPage(_pPage)
               ,m_bFirstActivation( sal_True )
               ,m_bAttemptedFormCreation( false )
{
    DBG_CTOR(FmFormPageImpl,NULL);

    OSL_ENSURE( false, "FmFormPageImpl::FmFormPageImpl: I'm pretty sure the below code isn't valid anymore ..." );
    // streaming of form/controls is not a supported operation anymore, in that it is not guaranteed
    // that really everything is copied. XCloneable should be used instead.

    // copy it by streaming
    // creating a pipe
    Reference< ::com::sun::star::io::XOutputStream >  xOutPipe(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.Pipe")), UNO_QUERY);
    Reference< ::com::sun::star::io::XInputStream >  xInPipe(xOutPipe, UNO_QUERY);

    // creating the mark streams
    Reference< ::com::sun::star::io::XInputStream >  xMarkIn(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")), UNO_QUERY);
    Reference< ::com::sun::star::io::XActiveDataSink >  xMarkSink(xMarkIn, UNO_QUERY);

    Reference< ::com::sun::star::io::XOutputStream >  xMarkOut(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")), UNO_QUERY);
    Reference< ::com::sun::star::io::XActiveDataSource >  xMarkSource(xMarkOut, UNO_QUERY);

    // connect pipe and sink
    Reference< ::com::sun::star::io::XActiveDataSink >  xSink(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), UNO_QUERY);

    // connect pipe and source
    Reference< ::com::sun::star::io::XActiveDataSource >  xSource(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")), UNO_QUERY);

    Reference< ::com::sun::star::io::XObjectOutputStream >  xOutStrm(xSource, UNO_QUERY);
    Reference< ::com::sun::star::io::XObjectInputStream >  xInStrm(xSink, UNO_QUERY);

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
const Reference< XNameContainer >& FmFormPageImpl::getForms( bool _bForceCreate )
{
    if ( m_xForms.is() || !_bForceCreate )
        return m_xForms;

    if ( !m_bAttemptedFormCreation )
    {
        m_bAttemptedFormCreation = true;

        const ::rtl::OUString sFormsCollectionServiceName = ::rtl::OUString::createFromAscii("com.sun.star.form.Forms");
        m_xForms = Reference< XNameContainer > (
            ::comphelper::getProcessServiceFactory()->createInstance( sFormsCollectionServiceName ),
            UNO_QUERY
        );
        DBG_ASSERT( m_xForms.is(), "FmFormPageImpl::getForms: could not create a forms collection!" );

        if ( m_aFormsCreationHdl.IsSet() )
        {
            m_aFormsCreationHdl.Call( this );
        }

        FmFormModel* pFormsModel = pPage ? PTR_CAST( FmFormModel, pPage->GetModel() ) : NULL;

        // give the newly created collection a place in the universe
        Reference< XChild > xAsChild( m_xForms, UNO_QUERY );
        if ( xAsChild.is() )
        {
            SfxObjectShell* pObjShell = pFormsModel ? pFormsModel->GetObjectShell() : NULL;
            if ( pObjShell )
                xAsChild->setParent( pObjShell->GetModel() );
        }

        // tell the UNDO environment that we have a new forms collection
        if ( pFormsModel )
            pFormsModel->GetUndoEnv().AddForms( m_xForms );
    }
    return m_xForms;
}

//------------------------------------------------------------------------------
FmFormPageImpl::~FmFormPageImpl()
{
    xCurrentForm = NULL;

    ::comphelper::disposeComponent( m_xForms );
    DBG_DTOR(FmFormPageImpl,NULL);
}

//------------------------------------------------------------------------------
bool FmFormPageImpl::validateCurForm()
{
    if ( !xCurrentForm.is() )
        return false;

    Reference< XChild > xAsChild( xCurrentForm, UNO_QUERY );
    DBG_ASSERT( xAsChild.is(), "FmFormPageImpl::validateCurForm: a form which is no child??" );
    if ( !xAsChild.is() || !xAsChild->getParent().is() )
        xCurrentForm.clear();

    return xCurrentForm.is();
}

//------------------------------------------------------------------------------
void FmFormPageImpl::setCurForm(Reference< ::com::sun::star::form::XForm >  xForm)
{
    xCurrentForm = xForm;
}

//------------------------------------------------------------------------------
Reference< XForm >  FmFormPageImpl::getDefaultForm()
{
    Reference< XForm > xForm;

    Reference< XNameContainer > xForms( getForms() );

    // by default, we use our "current form"
    if ( !validateCurForm() )
    {
        // check whether there is a "standard" form
        if ( xForms->hasElements() )
        {
            // suche die Standardform
            ::rtl::OUString sStandardFormname = String( SVX_RES( RID_STR_STDFORMNAME ) );

            try
            {
                if ( xForms->hasByName( sStandardFormname ) )
                    xForm.set( xForms->getByName( sStandardFormname ), UNO_QUERY_THROW );
                else
                {
                    Reference< XIndexAccess > xFormsByIndex( xForms, UNO_QUERY_THROW );
                    xForm.set( xFormsByIndex->getByIndex(0), UNO_QUERY_THROW );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
    else
    {
        xForm = xCurrentForm;
    }

    // did not find an existing suitable form -> create a new one
    if ( !xForm.is() )
    {
        SdrModel* pModel = pPage->GetModel();
        XubString aStr(SVX_RES(RID_STR_FORM));
        XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
        aUndoStr.SearchAndReplace('#', aStr);
        pModel->BegUndo(aUndoStr);

        try
        {
            xForm.set( ::comphelper::getProcessServiceFactory()->createInstance( FM_SUN_COMPONENT_FORM ), UNO_QUERY );

            // a form should always have the command type table as default
            Reference< XPropertySet > xFormProps( xForm, UNO_QUERY_THROW );
            xFormProps->setPropertyValue( FM_PROP_COMMANDTYPE, makeAny( sal_Int32( CommandType::TABLE ) ) );

            // and the "Standard" name
            ::rtl::OUString sName = String( SVX_RES( RID_STR_STDFORMNAME ) );
            xFormProps->setPropertyValue( FM_PROP_NAME, makeAny( sName ) );

            Reference< XIndexContainer > xContainer( xForms, UNO_QUERY );
            pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                       FmUndoContainerAction::Inserted,
                                                       xContainer,
                                                       xForm,
                                                       xContainer->getCount()));
            xForms->insertByName( sName, makeAny( xForm ) );
            xCurrentForm = xForm;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            xForm.clear();
        }

        pModel->EndUndo();
    }

    return xForm;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::findPlaceInFormComponentHierarchy(
    const Reference< XFormComponent > & rContent, const Reference< XDataSource > & rDatabase,
    const ::rtl::OUString& rDBTitle, const ::rtl::OUString& rCursorSource, sal_Int32 nCommandType )
{
    // if the control already is child of a form, don't do anything
    if (!rContent.is() || rContent->getParent().is())
        return NULL;

    Reference< XForm >  xForm;

    // Wenn Datenbank und CursorSource gesetzt sind, dann wird
    // die Form anhand dieser Kriterien gesucht, ansonsten nur aktuelle
    // und die StandardForm
    if (rDatabase.is() && rCursorSource.getLength())
    {
        validateCurForm();

        // erst in der aktuellen form suchen
        xForm = findFormForDataSource( xCurrentForm, rDatabase, rCursorSource, nCommandType );

        Reference< ::com::sun::star::container::XIndexAccess >  xFormsByIndex( getForms(), UNO_QUERY );
        DBG_ASSERT(xFormsByIndex.is(), "FmFormPageImpl::findPlaceInFormComponentHierarchy : no index access for my forms collection !");
        sal_Int32 nCount = xFormsByIndex->getCount();
        for (sal_Int32 i = 0; !xForm.is() && i < nCount; i++)
        {
            Reference< ::com::sun::star::form::XForm >  xToSearch;
            xFormsByIndex->getByIndex(i) >>= xToSearch;
            xForm = findFormForDataSource( xToSearch, rDatabase, rCursorSource, nCommandType );
        }

        // wenn keine ::com::sun::star::form gefunden, dann eine neue erzeugen
        if (!xForm.is())
        {
            SdrModel* pModel = pPage->GetModel();
            XubString aStr(SVX_RES(RID_STR_FORM));
            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
            aUndoStr.SearchAndReplace('#', aStr);
            pModel->BegUndo(aUndoStr);
            xForm = Reference< ::com::sun::star::form::XForm >(::comphelper::getProcessServiceFactory()->createInstance(FM_SUN_COMPONENT_FORM), UNO_QUERY);
            // a form should always have the command type table as default
            Reference< ::com::sun::star::beans::XPropertySet > xFormProps(xForm, UNO_QUERY);
            try { xFormProps->setPropertyValue(FM_PROP_COMMANDTYPE, makeAny(sal_Int32(CommandType::TABLE))); }
            catch(Exception&) { }

            if (rDBTitle.getLength())
                xFormProps->setPropertyValue(FM_PROP_DATASOURCE,makeAny(rDBTitle));
            else
            {
                Reference< ::com::sun::star::beans::XPropertySet >  xDatabaseProps(rDatabase, UNO_QUERY);
                Any aDatabaseUrl = xDatabaseProps->getPropertyValue(FM_PROP_URL);
                xFormProps->setPropertyValue(FM_PROP_DATASOURCE, aDatabaseUrl);
            }

            xFormProps->setPropertyValue(FM_PROP_COMMAND,makeAny(rCursorSource));
            xFormProps->setPropertyValue(FM_PROP_COMMANDTYPE, makeAny(nCommandType));

            Reference< ::com::sun::star::container::XNameAccess >  xNamedSet( getForms(), UNO_QUERY );
            ::rtl::OUString aName;

            if ((CommandType::TABLE == nCommandType) || (CommandType::QUERY == nCommandType))
            {
                // Namen der ::com::sun::star::form ueber den Titel der CursorSource setzen
                aName = getUniqueName(rCursorSource, xNamedSet);
            }
            else
                // ansonsten StandardformName verwenden
                aName = getUniqueName(::rtl::OUString(String(SVX_RES(RID_STR_STDFORMNAME))), xNamedSet);

            xFormProps->setPropertyValue(FM_PROP_NAME, makeAny(aName));

            Reference< ::com::sun::star::container::XIndexContainer >  xContainer( getForms(), UNO_QUERY );
            pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                     FmUndoContainerAction::Inserted,
                                                     xContainer,
                                                     xForm,
                                                     xContainer->getCount()));


            getForms()->insertByName(aName, makeAny(xForm));
            pModel->EndUndo();
        }
        xCurrentForm = xForm;
    }

    xForm = getDefaultForm();
    return xForm;
}

//------------------------------------------------------------------------------
Reference< XForm >  FmFormPageImpl::findFormForDataSource(
        const Reference< XForm > & rForm, const Reference< XDataSource > & _rxDatabase,
        const ::rtl::OUString& _rCursorSource, sal_Int32 nCommandType)
{
    Reference< XForm >          xResultForm;
    Reference< XRowSet >        xDBForm(rForm, UNO_QUERY);
    Reference< XPropertySet >   xFormProps(rForm, UNO_QUERY);
    if (!xDBForm.is() || !xFormProps.is())
        return xResultForm;

    OSL_ENSURE(_rxDatabase.is(), "FmFormPageImpl::findFormForDataSource: invalid data source!");
    ::rtl::OUString sLookupName;            // the name of the data source we're looking for
    ::rtl::OUString sFormDataSourceName;    // the name of the data source the current connection in the form is based on
    try
    {
        Reference< XPropertySet > xDSProps(_rxDatabase, UNO_QUERY);
        if (xDSProps.is())
            xDSProps->getPropertyValue(FM_PROP_NAME) >>= sLookupName;

        xFormProps->getPropertyValue(FM_PROP_DATASOURCE) >>= sFormDataSourceName;
        // if there's no DataSourceName set at the form, check whether we can deduce one from its
        // ActiveConnection
        if (0 == sFormDataSourceName.getLength())
        {
            Reference< XConnection > xFormConnection;
            xFormProps->getPropertyValue( FM_PROP_ACTIVE_CONNECTION ) >>= xFormConnection;
            if ( !xFormConnection.is() )
                OStaticDataAccessTools().isEmbeddedInDatabase( xFormProps, xFormConnection );
            if (xFormConnection.is())
            {
                Reference< XChild > xConnAsChild(xFormConnection, UNO_QUERY);
                if (xConnAsChild.is())
                {
                    Reference< XDataSource > xFormDS(xConnAsChild->getParent(), UNO_QUERY);
                    if (xFormDS.is())
                    {
                        xDSProps = xDSProps.query(xFormDS);
                        if (xDSProps.is())
                            xDSProps->getPropertyValue(FM_PROP_NAME) >>= sFormDataSourceName;
                    }
                }
            }
        }
    }
    catch(const Exception& e)
    {
        (void)e;
        OSL_ENSURE(sal_False, "FmFormPageImpl::findFormForDataSource: caught an exception!");
    }

    if (sLookupName == sFormDataSourceName)
    {
        // jetzt noch ueberpruefen ob CursorSource und Type uebereinstimmen
        ::rtl::OUString aCursorSource = ::comphelper::getString(xFormProps->getPropertyValue(FM_PROP_COMMAND));
        sal_Int32 nType = ::comphelper::getINT32(xFormProps->getPropertyValue(FM_PROP_COMMANDTYPE));
        if (!aCursorSource.getLength() || ((nType == nCommandType) && (aCursorSource == _rCursorSource))) // found the form
        {
            xResultForm = rForm;
            // Ist noch keine Datenquelle gesetzt, wird dieses hier nachgeholt
            if (!aCursorSource.getLength())
            {
                xFormProps->setPropertyValue(FM_PROP_COMMAND, makeAny(_rCursorSource));
                xFormProps->setPropertyValue(FM_PROP_COMMANDTYPE, makeAny((sal_Int32)nCommandType));
            }
        }
    }

    // as long as xResultForm is NULL, search the child forms of rForm
    Reference< XIndexAccess >  xComponents(rForm, UNO_QUERY);
    sal_Int32 nCount = xComponents->getCount();
    for (sal_Int32 i = 0; !xResultForm.is() && i < nCount; ++i)
    {
        Reference< ::com::sun::star::form::XForm >  xSearchForm;
        xComponents->getByIndex(i) >>= xSearchForm;
        // continue searching in the sub form
        if (xSearchForm.is())
            xResultForm = findFormForDataSource( xSearchForm, _rxDatabase, _rCursorSource, nCommandType );
    }
    return xResultForm;
}

//------------------------------------------------------------------------------
::rtl::OUString FmFormPageImpl::setUniqueName(const Reference< XFormComponent > & xFormComponent, const Reference< XForm > & xControls)
{
    ::rtl::OUString sName;
    Reference< ::com::sun::star::beans::XPropertySet >  xSet(xFormComponent, UNO_QUERY);
    if (xSet.is())
    {
        sName = ::comphelper::getString( xSet->getPropertyValue( FM_PROP_NAME ) );
        Reference< ::com::sun::star::container::XNameAccess >  xNameAcc(xControls, UNO_QUERY);

        if (!sName.getLength() || xNameAcc->hasByName(sName))
        {
            // setzen eines default Namens ueber die ClassId
            sal_Int16 nClassId( FormComponentType::CONTROL );
            xSet->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;
            Reference< XServiceInfo > xSI( xSet, UNO_QUERY );

            ::rtl::OUString sDefaultName = getDefaultName( nClassId, xControls, xSI );
            // bei Radiobuttons, die einen Namen haben, diesen nicht ueberschreiben!
            if (!sName.getLength() || nClassId != ::com::sun::star::form::FormComponentType::RADIOBUTTON)
            {
                xSet->setPropertyValue(FM_PROP_NAME, makeAny(sDefaultName));
            }

            sName = sDefaultName;
        }
    }
    return sName;
}


UniString FmFormPageImpl::getDefaultName( sal_Int16 _nClassId, const Reference< XServiceInfo >& _rxObject )
{
    sal_uInt16 nResId;

    switch (_nClassId)
    {
        case FormComponentType::COMMANDBUTTON:  nResId = RID_STR_BUTTON_CLASSNAME;      break;
        case FormComponentType::RADIOBUTTON:    nResId = RID_STR_RADIOBUTTON_CLASSNAME; break;
        case FormComponentType::CHECKBOX:       nResId = RID_STR_CHECKBOX_CLASSNAME;    break;
        case FormComponentType::LISTBOX:        nResId = RID_STR_LISTBOX_CLASSNAME;     break;
        case FormComponentType::COMBOBOX:       nResId = RID_STR_COMBOBOX_CLASSNAME;    break;
        case FormComponentType::GROUPBOX:       nResId = RID_STR_GROUPBOX_CLASSNAME;    break;
        case FormComponentType::IMAGEBUTTON:    nResId = RID_STR_IMAGE_CLASSNAME;       break;
        case FormComponentType::FIXEDTEXT:      nResId = RID_STR_FIXEDTEXT_CLASSNAME;   break;
        case FormComponentType::GRIDCONTROL:    nResId = RID_STR_GRID_CLASSNAME;        break;
        case FormComponentType::FILECONTROL:    nResId = RID_STR_FILECONTROL_CLASSNAME; break;
        case FormComponentType::DATEFIELD:      nResId = RID_STR_DATEFIELD_CLASSNAME;   break;
        case FormComponentType::TIMEFIELD:      nResId = RID_STR_TIMEFIELD_CLASSNAME;   break;
        case FormComponentType::NUMERICFIELD:   nResId = RID_STR_NUMERICFIELD_CLASSNAME;    break;
        case FormComponentType::CURRENCYFIELD:  nResId = RID_STR_CURRENCYFIELD_CLASSNAME;   break;
        case FormComponentType::PATTERNFIELD:   nResId = RID_STR_PATTERNFIELD_CLASSNAME;    break;
        case FormComponentType::IMAGECONTROL:   nResId = RID_STR_IMAGECONTROL_CLASSNAME;    break;
        case FormComponentType::HIDDENCONTROL:  nResId = RID_STR_HIDDEN_CLASSNAME;      break;
        case FormComponentType::SCROLLBAR:      nResId = RID_STR_CLASSNAME_SCROLLBAR;   break;
        case FormComponentType::SPINBUTTON:     nResId = RID_STR_CLASSNAME_SPINBUTTON;  break;
        case FormComponentType::NAVIGATIONBAR:  nResId = RID_STR_NAVBAR_CLASSNAME;      break;

        case FormComponentType::TEXTFIELD:
            nResId = RID_STR_EDIT_CLASSNAME;
            if ( _rxObject.is() && _rxObject->supportsService( FM_SUN_COMPONENT_FORMATTEDFIELD ) )
                nResId = RID_STR_FORMATTED_CLASSNAME;
            break;

        default:
            nResId = RID_STR_CONTROL_CLASSNAME;     break;
    }

    return SVX_RES(nResId);
}

//------------------------------------------------------------------------------
::rtl::OUString FmFormPageImpl::getDefaultName(
    sal_Int16 _nClassId, const Reference< XForm >& _rxControls, const Reference< XServiceInfo >& _rxObject ) const
{
    ::rtl::OUString aClassName=getDefaultName( _nClassId, _rxObject );

    Reference< ::com::sun::star::container::XNameAccess >  xNamedSet( _rxControls, UNO_QUERY );
    return getUniqueName(aClassName, xNamedSet);
}

//------------------------------------------------------------------
::rtl::OUString FmFormPageImpl::getUniqueName(const ::rtl::OUString& rName, const Reference< ::com::sun::star::container::XNameAccess > & xNamedSet) const
{
    Reference< ::com::sun::star::container::XIndexAccess >  xIndexSet(xNamedSet, UNO_QUERY);
    ::rtl::OUString sName( rName );

    if ( !xIndexSet.is() )
        return sName;

    sal_Int32 n = 0;
    ::rtl::OUString sClassName = rName;

    while ( xNamedSet->hasByName( sName ) )
        sName = sClassName + ::rtl::OUString::valueOf(++n);

    return sName;
}

//------------------------------------------------------------------------------
void FmFormPageImpl::write(const Reference< ::com::sun::star::io::XObjectOutputStream > & xOutStrm) const
{
    Reference< ::com::sun::star::io::XMarkableStream >  xMarkStrm(xOutStrm, UNO_QUERY);
    if (!xMarkStrm.is())
        return; // exception

    //  sortieren der objectlist nach der Reihenfolge
    FmObjectList aList;
    fillList(aList, *pPage, sal_True);

    // schreiben aller forms
    Reference< ::com::sun::star::io::XPersistObject >  xAsPersist( const_cast< FmFormPageImpl* >( this )->getForms(), UNO_QUERY);
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
        Reference< ::com::sun::star::io::XPersistObject >  xObj(aList.GetObject(i)->GetUnoControlModel(), UNO_QUERY);
        if (xObj.is())
        {
            xOutStrm->writeObject(xObj);
        }
        else
        {
            ;// exception
        }
    }
}

//------------------------------------------------------------------------------
void FmFormPageImpl::read(const Reference< ::com::sun::star::io::XObjectInputStream > & xInStrm)
{
    Reference< ::com::sun::star::io::XMarkableStream >  xMarkStrm(xInStrm, UNO_QUERY);
    if (!xMarkStrm.is())
        return; // exception

    //  sortieren der objectlist nach der Reihenfolge
    FmObjectList aList;
    fillList(aList, *pPage, sal_False);

    // lesen aller forms
    Reference< ::com::sun::star::io::XPersistObject >  xAsPersist( getForms(), UNO_QUERY );
    if (xAsPersist.is())
        xAsPersist->read(xInStrm);
        // don't use the readObject of the stream, as this wouldn't be compatible with older documents

    // Zuordnung der Formobjekte zu den FormComponents
    sal_Int32 nLength = xInStrm->readLong();
    DBG_ASSERT(nLength == (sal_Int32) aList.Count(), "Fehler beim Lesen der UnoModels");
    for (sal_Int32 i = 0; i < nLength; i++)
    {
        Reference< ::com::sun::star::awt::XControlModel >  xRef(xInStrm->readObject(), UNO_QUERY);
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


