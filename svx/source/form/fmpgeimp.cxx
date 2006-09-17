/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmpgeimp.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:08:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::svxform;

DBG_NAME(FmFormPageImpl);
//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage)
               :pPage(_pPage)
               ,m_bFirstActivation( sal_True )
               ,m_bAttemptedFormCreation( sal_False )
{
    DBG_CTOR(FmFormPageImpl,NULL);
}

//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl(FmFormPage* _pPage, const FmFormPageImpl& rImpl)
               :pPage(_pPage)
               ,m_bFirstActivation( sal_True )
               ,m_bAttemptedFormCreation( sal_False )
{
    DBG_CTOR(FmFormPageImpl,NULL);

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
        m_bAttemptedFormCreation = sal_True;

        const ::rtl::OUString sFormsCollectionServiceName = ::rtl::OUString::createFromAscii("com.sun.star.form.Forms");
        m_xForms = Reference< XNameContainer > (
            ::comphelper::getProcessServiceFactory()->createInstance( sFormsCollectionServiceName ),
            UNO_QUERY
        );
        DBG_ASSERT( m_xForms.is(), "FmFormPageImpl::getForms: could not create a forms collection!" );

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
void FmFormPageImpl::validateCurForm()
{
    if ( !xCurrentForm.is() )
        return;

    Reference< XChild > xAsChild( xCurrentForm, UNO_QUERY );
    DBG_ASSERT( xAsChild.is(), "FmFormPageImpl::validateCurForm: a form which is no child??" );
    if ( !xAsChild.is() || !xAsChild->getParent().is() )
        xCurrentForm.clear();
}

//------------------------------------------------------------------------------
void FmFormPageImpl::setCurForm(Reference< ::com::sun::star::form::XForm >  xForm)
{
    xCurrentForm = xForm;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::getDefaultForm()
{
    Reference< ::com::sun::star::form::XForm >  xForm;

    try
    {
        Reference< XNameContainer > xForms( getForms() );

        validateCurForm();

        // wenn noch kein TargetForm gefunden, dann aktuelle oder Default
        if (!xCurrentForm.is())
        {
            if (xForms->hasElements())
            {
                // suche die Standardform
                ::rtl::OUString ustrStdFormName = ::rtl::OUString(String(SVX_RES(RID_STR_STDFORMNAME)));

                if (xForms->hasByName(ustrStdFormName))
                {
                    try
                    {
                        xForms->getByName(ustrStdFormName) >>= xForm;
                    }
                    catch(::com::sun::star::container::NoSuchElementException &)
                    {
                        DBG_ERROR("NoSuchElementException occured!");
                    }
                    catch(::com::sun::star::lang::WrappedTargetException &)
                    {
                        DBG_ERROR("WrappedTargetException occured!");
                    }

                }

                // gibt es denn ueberhaupt eine
                if (!xForm.is())
                {
                    Reference< ::com::sun::star::container::XIndexAccess >  xGetFirst(xForms, UNO_QUERY);
                    DBG_ASSERT(xGetFirst.is(), "FmFormPageImpl::getDefaultForm : no IndexAccess on my form container !");
                        // wenn das anspringt, muesste man sich die Namen des NameContainers geben lassen und dann das Objekt fuer den
                        // ersten Namen erfragen ... aber normalerweise sollte die FOrms-Sammlung auch einen IndexAccess haben
                    xGetFirst->getByIndex(0) >>= xForm;
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

            xForm = Reference< ::com::sun::star::form::XForm >(::comphelper::getProcessServiceFactory()->createInstance(FM_SUN_COMPONENT_FORM), UNO_QUERY);
            // a form should always have the command type table as default
            Reference< ::com::sun::star::beans::XPropertySet >  xSet(xForm, UNO_QUERY);
            try
            {
                xSet->setPropertyValue(FM_PROP_COMMANDTYPE, makeAny(sal_Int32(CommandType::TABLE)));
            }
            catch(Exception&)
            {
            }

            ::rtl::OUString aName = String(SVX_RES(RID_STR_STDFORMNAME));
            xSet->setPropertyValue(FM_PROP_NAME, makeAny(aName));


            Reference< ::com::sun::star::container::XIndexContainer >  xContainer(xForms, UNO_QUERY);
            pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                       FmUndoContainerAction::Inserted,
                                                       xContainer,
                                                       xForm,
                                                       xContainer->getCount()));
            xForms->insertByName(aName, makeAny(xForm));
            xCurrentForm = xForm;
            pModel->EndUndo();
        }
    }
    catch( const Exception& )
    {
        DBG_ERROR( "FmFormPageImpl::getDefaultForm: caught an exception!" );
        xForm.clear();
    }

    return xForm;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::placeInFormComponentHierarchy(
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
        DBG_ASSERT(xFormsByIndex.is(), "FmFormPageImpl::placeInFormComponentHierarchy : no index access for my forms collection !");
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
    // eindeutigen Namen fuer die Componente setzen
    setUniqueName(rContent, xForm);
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
        if (0 == sFormDataSourceName.getLength())
        {
            // check if it has an active connection
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
                        // the data source which created the connection
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
::rtl::OUString FmFormPageImpl::setUniqueName(const Reference< ::com::sun::star::form::XFormComponent > & xFormComponent, const Reference< ::com::sun::star::form::XForm > & xControls)
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
                ::rtl::OUString aText;
                xSet->getPropertyValue( FM_PROP_LABEL ) >>= aText;
                if (!aText.getLength())
                {
                    aLabel.SearchAndReplace( getDefaultName( nClassId, xSI ), ::rtl::OUString(String(SVX_RES(nResId)) ));
                    xSet->setPropertyValue( FM_PROP_LABEL, makeAny(::rtl::OUString(aLabel)) );
                }
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
            ;// exception
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


