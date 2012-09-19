/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "svx/svxerr.hxx"
#include "fmpgeimp.hxx"
#include "fmundo.hxx"
#include "svx/fmtools.hxx"
#include "fmprop.hrc"
#include "fmservs.hxx"
#include "fmobj.hxx"
#include "formcontrolfactory.hxx"
#include "svx/svditer.hxx"
#include "svx/fmresids.hrc"
#include "svx/dbtoolsclient.hxx"
#include "treevisitor.hxx"

#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/container/EnumerableMap.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>

#include <sfx2/objsh.hxx>
#include <svx/fmglob.hxx>
#include <svx/fmpage.hxx>
#include <svx/fmmodel.hxx>
#include <tools/resid.hxx>
#include <tools/diagnose_ex.h>
#include <tools/shl.hxx>
#include <vcl/stdtext.hxx>
#include <svx/dialmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <unotools/streamwrap.hxx>
#include <rtl/logfile.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::container::XMap;
using ::com::sun::star::container::EnumerableMap;
using ::com::sun::star::drawing::XControlShape;
using namespace ::svxform;

DBG_NAME(FmFormPageImpl)
//------------------------------------------------------------------------------
FmFormPageImpl::FmFormPageImpl( FmFormPage& _rPage )
               :m_rPage( _rPage )
               ,m_bFirstActivation( sal_True )
               ,m_bAttemptedFormCreation( false )
               ,m_bInFind( false )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::FmFormPageImpl" );
    DBG_CTOR(FmFormPageImpl,NULL);
}

//------------------------------------------------------------------------------
namespace
{
    typedef Reference< XInterface > FormComponent;

    class FormComponentInfo
    {
    public:
        size_t childCount( const FormComponent& _component ) const
        {
            Reference< XIndexAccess > xContainer( _component, UNO_QUERY );
            if ( xContainer.is() )
                return xContainer->getCount();
            return 0;
        }

        FormComponent getChild( const FormComponent& _component, size_t _index ) const
        {
            Reference< XIndexAccess > xContainer( _component, UNO_QUERY_THROW );
            return FormComponent( xContainer->getByIndex( _index ), UNO_QUERY );
        }
    };

    typedef ::std::pair< FormComponent, FormComponent > FormComponentPair;

    class FormHierarchyComparator
    {
    public:
        FormHierarchyComparator()
        {
        }

        size_t childCount( const FormComponentPair& _components ) const
        {
            size_t lhsCount = m_aComponentInfo.childCount( _components.first );
            size_t rhsCount = m_aComponentInfo.childCount( _components.second );
            if  ( lhsCount != rhsCount )
                throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Found inconsistent form component hierarchies (1)!" ) ), NULL );
            return lhsCount;
        }

        FormComponentPair getChild( const FormComponentPair& _components, size_t _index ) const
        {
            return FormComponentPair(
                m_aComponentInfo.getChild( _components.first, _index ),
                m_aComponentInfo.getChild( _components.second, _index )
            );
        }
    private:
        FormComponentInfo   m_aComponentInfo;
    };

    typedef ::std::map< Reference< XControlModel >, Reference< XControlModel >, ::comphelper::OInterfaceCompare< XControlModel > > MapControlModels;

    class FormComponentAssignment
    {
    public:
        FormComponentAssignment( MapControlModels& _out_controlModelMap )
            :m_rControlModelMap( _out_controlModelMap )
        {
        }

        void    process( const FormComponentPair& _component )
        {
            Reference< XControlModel > lhsControlModel( _component.first, UNO_QUERY );
            Reference< XControlModel > rhsControlModel( _component.second, UNO_QUERY );
            if ( lhsControlModel.is() != rhsControlModel.is() )
                throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Found inconsistent form component hierarchies (2)!" ) ), NULL );

            if ( lhsControlModel.is() )
                m_rControlModelMap[ lhsControlModel ] = rhsControlModel;
        }

    private:
        MapControlModels&   m_rControlModelMap;
    };
}

//------------------------------------------------------------------------------
void FmFormPageImpl::initFrom( FmFormPageImpl& i_foreignImpl )
{
    // clone the Forms collection
    const Reference< XNameContainer > xForeignForms( const_cast< FmFormPageImpl& >( i_foreignImpl ).getForms( false ) );
    const Reference< XCloneable > xCloneable( xForeignForms, UNO_QUERY );
    if ( !xCloneable.is() )
    {
        // great, nothing to do
        OSL_ENSURE( !xForeignForms.is(), "FmFormPageImpl::FmFormPageImpl: a non-cloneable forms container!?" );
        return;
    }

    try
    {
        m_xForms.set( xCloneable->createClone(), UNO_QUERY_THROW );

        // create a mapping between the original control models and their clones
        MapControlModels aModelAssignment;

        typedef TreeVisitor< FormComponentPair, FormHierarchyComparator, FormComponentAssignment >   FormComponentVisitor;
        FormComponentVisitor aVisitor = FormComponentVisitor( FormHierarchyComparator() );

        FormComponentAssignment aAssignmentProcessor( aModelAssignment );
        aVisitor.process( FormComponentPair( xCloneable, m_xForms ), aAssignmentProcessor );

        // assign the cloned models to their SdrObjects
        SdrObjListIter aForeignIter( i_foreignImpl.m_rPage );
        SdrObjListIter aOwnIter( m_rPage );

        OSL_ENSURE( aForeignIter.IsMore() == aOwnIter.IsMore(), "FmFormPageImpl::FmFormPageImpl: inconsistent number of objects (1)!" );
        while ( aForeignIter.IsMore() && aOwnIter.IsMore() )
        {
            FmFormObj* pForeignObj = dynamic_cast< FmFormObj* >( aForeignIter.Next() );
            FmFormObj* pOwnObj = dynamic_cast< FmFormObj* >( aOwnIter.Next() );

            bool bForeignIsForm = pForeignObj && ( pForeignObj->GetObjInventor() == FmFormInventor );
            bool bOwnIsForm = pOwnObj && ( pOwnObj->GetObjInventor() == FmFormInventor );

            if ( bForeignIsForm != bOwnIsForm )
            {
                // if this fires, don't attempt to do further assignments, something's completely messed up
                SAL_WARN( "svx.form", "FmFormPageImpl::FmFormPageImpl: inconsistent ordering of objects!" );
                break;
            }

            if ( !bForeignIsForm )
                // no form control -> next round
                continue;

            Reference< XControlModel > xForeignModel( pForeignObj->GetUnoControlModel() );
            if ( !xForeignModel.is() )
            {
                // if this fires, the SdrObject does not have a UNO Control Model. This is pathological, but well ...
                // So the cloned SdrObject will also not have a UNO Control Model.
                SAL_WARN( "svx.form", "FmFormPageImpl::FmFormPageImpl: control shape without control!" );
                continue;
            }

            MapControlModels::const_iterator assignment = aModelAssignment.find( xForeignModel );
            if ( assignment == aModelAssignment.end() )
            {
                // if this fires, the source SdrObject has a model, but it is not part of the model hierarchy in
                // i_foreignImpl.getForms().
                // Pathological, too ...
                SAL_WARN( "svx.form", "FmFormPageImpl::FmFormPageImpl: no clone found for this model!" );
                continue;
            }

            pOwnObj->SetUnoControlModel( assignment->second );
        }
        OSL_ENSURE( aForeignIter.IsMore() == aOwnIter.IsMore(), "FmFormPageImpl::FmFormPageImpl: inconsistent number of objects (2)!" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
Reference< XMap > FmFormPageImpl::getControlToShapeMap()
{
    Reference< XMap > xControlShapeMap( m_aControlShapeMap.get(), UNO_QUERY );
    if ( xControlShapeMap.is() )
        return xControlShapeMap;

    xControlShapeMap = impl_createControlShapeMap_nothrow();
    m_aControlShapeMap = xControlShapeMap;
    return xControlShapeMap;
}

//------------------------------------------------------------------------------
namespace
{
    static void lcl_insertFormObject_throw( const FmFormObj& _object, const Reference< XMap >& _map )
    {
        // the control model
        Reference< XControlModel > xControlModel( _object.GetUnoControlModel(), UNO_QUERY );
        OSL_ENSURE( xControlModel.is(), "lcl_insertFormObject_throw: suspicious: no control model!" );
        if ( !xControlModel.is() )
            return;

        Reference< XControlShape > xControlShape( const_cast< FmFormObj& >( _object ).getUnoShape(), UNO_QUERY );
        OSL_ENSURE( xControlShape.is(), "lcl_insertFormObject_throw: suspicious: no control shape!" );
        if ( !xControlShape.is() )
            return;

        _map->put( makeAny( xControlModel ), makeAny( xControlShape ) );
    }

    static void lcl_removeFormObject_throw( const FmFormObj& _object, const Reference< XMap >& _map, bool i_ignoreNonExistence = false )
    {
        // the control model
        Reference< XControlModel > xControlModel( _object.GetUnoControlModel(), UNO_QUERY );
        OSL_ENSURE( xControlModel.is(), "lcl_removeFormObject: suspicious: no control model!" );
        if ( !xControlModel.is() )
            return;

    #if OSL_DEBUG_LEVEL > 0
        Any aOldAssignment =
    #endif
            _map->remove( makeAny( xControlModel ) );
    #if OSL_DEBUG_LEVEL > 0
        (void)aOldAssignment;
    #endif
        OSL_ENSURE( !i_ignoreNonExistence ||
            ( aOldAssignment == makeAny( Reference< XControlShape >( const_cast< FmFormObj& >( _object ).getUnoShape(), UNO_QUERY ) ) ),
                "lcl_removeFormObject: map was inconsistent!" );
        (void)i_ignoreNonExistence;
    }
}

//------------------------------------------------------------------------------
Reference< XMap > FmFormPageImpl::impl_createControlShapeMap_nothrow()
{
    Reference< XMap > xMap;

    try
    {
        xMap.set( EnumerableMap::create( comphelper::getProcessComponentContext(),
            ::cppu::UnoType< XControlModel >::get(),
            ::cppu::UnoType< XControlShape >::get()
        ).get(), UNO_SET_THROW );

        SdrObjListIter aPageIter( m_rPage );
        while ( aPageIter.IsMore() )
        {
            // only FmFormObjs are what we're interested in
            FmFormObj* pCurrent = FmFormObj::GetFormObject( aPageIter.Next() );
            if ( !pCurrent )
                continue;

            lcl_insertFormObject_throw( *pCurrent, xMap );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xMap;
}

//------------------------------------------------------------------------------
const Reference< XNameContainer >& FmFormPageImpl::getForms( bool _bForceCreate )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::getForms" );
    if ( m_xForms.is() || !_bForceCreate )
        return m_xForms;

    if ( !m_bAttemptedFormCreation )
    {
        m_bAttemptedFormCreation = true;

        const ::rtl::OUString sFormsCollectionServiceName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.Forms") );
        m_xForms = Reference< XNameContainer > (
            ::comphelper::getProcessServiceFactory()->createInstance( sFormsCollectionServiceName ),
            UNO_QUERY
        );
        DBG_ASSERT( m_xForms.is(), "FmFormPageImpl::getForms: could not create a forms collection!" );

        if ( m_aFormsCreationHdl.IsSet() )
        {
            m_aFormsCreationHdl.Call( this );
        }

        FmFormModel* pFormsModel = PTR_CAST( FmFormModel, m_rPage.GetModel() );

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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::validateCurForm" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::setCurForm" );
    xCurrentForm = xForm;
}

//------------------------------------------------------------------------------
Reference< XForm >  FmFormPageImpl::getDefaultForm()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::getDefaultForm" );
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
        SdrModel* pModel = m_rPage.GetModel();

        if( pModel->IsUndoEnabled() )
        {
            XubString aStr(SVX_RES(RID_STR_FORM));
            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
            aUndoStr.SearchAndReplace(rtl::OUString('#'), aStr);
            pModel->BegUndo(aUndoStr);
        }

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
            if( pModel->IsUndoEnabled() )
            {
                pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                           FmUndoContainerAction::Inserted,
                                                           xContainer,
                                                           xForm,
                                                           xContainer->getCount()));
            }
            xForms->insertByName( sName, makeAny( xForm ) );
            xCurrentForm = xForm;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            xForm.clear();
        }

        if( pModel->IsUndoEnabled() )
            pModel->EndUndo();
    }

    return xForm;
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XForm >  FmFormPageImpl::findPlaceInFormComponentHierarchy(
    const Reference< XFormComponent > & rContent, const Reference< XDataSource > & rDatabase,
    const ::rtl::OUString& rDBTitle, const ::rtl::OUString& rCursorSource, sal_Int32 nCommandType )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::findPlaceInFormComponentHierarchy" );
    // if the control already is child of a form, don't do anything
    if (!rContent.is() || rContent->getParent().is())
        return NULL;

    Reference< XForm >  xForm;

    // Wenn Datenbank und CursorSource gesetzt sind, dann wird
    // die Form anhand dieser Kriterien gesucht, ansonsten nur aktuelle
    // und die StandardForm
    if (rDatabase.is() && !rCursorSource.isEmpty())
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
            SdrModel* pModel = m_rPage.GetModel();

            const bool bUndo = pModel->IsUndoEnabled();

            if( bUndo )
            {
                XubString aStr(SVX_RES(RID_STR_FORM));
                XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                aUndoStr.SearchAndReplace(rtl::OUString('#'), aStr);
                pModel->BegUndo(aUndoStr);
            }

            xForm = Reference< ::com::sun::star::form::XForm >(::comphelper::getProcessServiceFactory()->createInstance(FM_SUN_COMPONENT_FORM), UNO_QUERY);
            // a form should always have the command type table as default
            Reference< ::com::sun::star::beans::XPropertySet > xFormProps(xForm, UNO_QUERY);
            try { xFormProps->setPropertyValue(FM_PROP_COMMANDTYPE, makeAny(sal_Int32(CommandType::TABLE))); }
            catch(Exception&) { }

            if (!rDBTitle.isEmpty())
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

            const bool bTableOrQuery = ( CommandType::TABLE == nCommandType ) || ( CommandType::QUERY == nCommandType );
            ::rtl::OUString sName = FormControlFactory::getUniqueName( xNamedSet,
                bTableOrQuery ? rCursorSource : ::rtl::OUString( String( SVX_RES( RID_STR_STDFORMNAME ) ) ) );

            xFormProps->setPropertyValue( FM_PROP_NAME, makeAny( sName ) );

            if( bUndo )
            {
                Reference< ::com::sun::star::container::XIndexContainer >  xContainer( getForms(), UNO_QUERY );
                pModel->AddUndo(new FmUndoContainerAction(*(FmFormModel*)pModel,
                                                         FmUndoContainerAction::Inserted,
                                                         xContainer,
                                                         xForm,
                                                         xContainer->getCount()));
            }

            getForms()->insertByName( sName, makeAny( xForm ) );

            if( bUndo )
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPageImpl::findFormForDataSource" );
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
        if (sFormDataSourceName.isEmpty())
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
        OSL_FAIL("FmFormPageImpl::findFormForDataSource: caught an exception!");
    }

    if (sLookupName == sFormDataSourceName)
    {
        // jetzt noch ueberpruefen ob CursorSource und Type uebereinstimmen
        ::rtl::OUString aCursorSource = ::comphelper::getString(xFormProps->getPropertyValue(FM_PROP_COMMAND));
        sal_Int32 nType = ::comphelper::getINT32(xFormProps->getPropertyValue(FM_PROP_COMMANDTYPE));
        if (aCursorSource.isEmpty() || ((nType == nCommandType) && (aCursorSource == _rCursorSource))) // found the form
        {
            xResultForm = rForm;
            // Ist noch keine Datenquelle gesetzt, wird dieses hier nachgeholt
            if (aCursorSource.isEmpty())
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
#if OSL_DEBUG_LEVEL > 0
    try
    {
        Reference< XChild > xChild( xFormComponent, UNO_QUERY_THROW );
        OSL_ENSURE( !xChild->getParent().is(), "FmFormPageImpl::setUniqueName: to be called before insertion!" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
#endif
    ::rtl::OUString sName;
    Reference< ::com::sun::star::beans::XPropertySet >  xSet(xFormComponent, UNO_QUERY);
    if (xSet.is())
    {
        sName = ::comphelper::getString( xSet->getPropertyValue( FM_PROP_NAME ) );
        Reference< ::com::sun::star::container::XNameAccess >  xNameAcc(xControls, UNO_QUERY);

        if (sName.isEmpty() || xNameAcc->hasByName(sName))
        {
            // setzen eines default Namens ueber die ClassId
            sal_Int16 nClassId( FormComponentType::CONTROL );
            xSet->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;

            ::rtl::OUString sDefaultName = FormControlFactory::getDefaultUniqueName_ByComponentType(
                Reference< XNameAccess >( xControls, UNO_QUERY ), xSet );

            // bei Radiobuttons, die einen Namen haben, diesen nicht ueberschreiben!
            if (sName.isEmpty() || nClassId != ::com::sun::star::form::FormComponentType::RADIOBUTTON)
            {
                xSet->setPropertyValue(FM_PROP_NAME, makeAny(sDefaultName));
            }

            sName = sDefaultName;
        }
    }
    return sName;
}

//----------------------------------------------------------------------------------------------------------------------
void FmFormPageImpl::formModelAssigned( const FmFormObj& _object )
{
    Reference< XMap > xControlShapeMap( m_aControlShapeMap.get(), UNO_QUERY );
    if ( !xControlShapeMap.is() )
        // our map does not exist -> not interested in this event
        return;

    try
    {
        lcl_removeFormObject_throw( _object,  xControlShapeMap, false );
        lcl_insertFormObject_throw( _object,  xControlShapeMap );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void FmFormPageImpl::formObjectInserted( const FmFormObj& _object )
{
    Reference< XMap > xControlShapeMap( m_aControlShapeMap.get(), UNO_QUERY );
    if ( !xControlShapeMap.is() )
        // our map does not exist -> not interested in this event
        return;

    try
    {
        lcl_insertFormObject_throw( _object,  xControlShapeMap );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void FmFormPageImpl::formObjectRemoved( const FmFormObj& _object )
{
    Reference< XMap > xControlShapeMap( m_aControlShapeMap.get(), UNO_QUERY );
    if ( !xControlShapeMap.is() )
        // our map does not exist -> not interested in this event
        return;

    try
    {
        lcl_removeFormObject_throw( _object, xControlShapeMap );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
