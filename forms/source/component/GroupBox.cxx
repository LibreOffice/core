/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GroupBox.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:26:51 $
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
#include "precompiled_forms.hxx"

#ifndef _FORMS_GROUPBOX_HXX_
#include "GroupBox.hxx"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//==================================================================
// OGroupBoxModel
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGroupBoxModel_CreateInstance(const Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OGroupBoxModel(_rxFactory));
}

//------------------------------------------------------------------
DBG_NAME( OGroupBoxModel )
//------------------------------------------------------------------
OGroupBoxModel::OGroupBoxModel(const Reference<starlang::XMultiServiceFactory>& _rxFactory)
    :OControlModel(_rxFactory, VCL_CONTROLMODEL_GROUPBOX, VCL_CONTROL_GROUPBOX)
{
    DBG_CTOR( OGroupBoxModel, NULL );
    m_nClassId = FormComponentType::GROUPBOX;
}

//------------------------------------------------------------------
OGroupBoxModel::OGroupBoxModel( const OGroupBoxModel* _pOriginal, const Reference<starlang::XMultiServiceFactory>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OGroupBoxModel, NULL );
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OGroupBoxModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_GROUPBOX;
    return aSupported;
}

//------------------------------------------------------------------
OGroupBoxModel::~OGroupBoxModel()
{
    DBG_DTOR( OGroupBoxModel, NULL );
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OGroupBoxModel )

//------------------------------------------------------------------------------
void OGroupBoxModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OControlModel::describeAggregateProperties( _rAggregateProps );
    // don't want to have the TabStop property
    RemoveProperty(_rAggregateProps, PROPERTY_TABSTOP);
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroupBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_GROUPBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OGroupBoxModel::write(const Reference< XObjectOutputStream>& _rxOutStream)
    throw(IOException, RuntimeException)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0002);
    writeHelpTextCompatibly(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OGroupBoxModel::read(const Reference< XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
{
    OControlModel::read( _rxInStream );

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OGroupBoxModel::read : version 0 ? this should never have been written !");
        // ups, ist das Englisch richtig ? ;)

    if (nVersion == 2)
        readHelpTextCompatibly(_rxInStream);

    if (nVersion > 0x0002)
    {
        DBG_ERROR("OGroupBoxModel::read : unknown version !");
    }
};

//==================================================================
// OGroupBoxControl
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGroupBoxControl_CreateInstance(const Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OGroupBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
OGroupBoxControl::OGroupBoxControl(const Reference<starlang::XMultiServiceFactory>& _rxFactory)
                   :OControl(_rxFactory, VCL_CONTROL_GROUPBOX)
{
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OGroupBoxControl::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_GROUPBOX;
    return aSupported;
}

//.........................................................................
}
//.........................................................................

