/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tableshape.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:10:27 $
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

#include "shapeimpl.hxx"
#include "svx/unoshprp.hxx"
#include "svx/svdotable.hxx"

///////////////////////////////////////////////////////////////////////

using ::rtl::OUString;

using namespace ::osl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::sdr::table;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

SvxTableShape::SvxTableShape( SdrObject* pObj ) throw()
:   SvxShape( pObj, aSvxMapProvider.GetMap(SVXMAP_TABLE) )
{
    SetShapeType( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.TableShape" ) ) );
}

//----------------------------------------------------------------------
SvxTableShape::~SvxTableShape() throw()
{
}

//----------------------------------------------------------------------

bool SvxTableShape::setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_TABLETEMPLATE:
    {
        Reference< XIndexAccess > xTemplate;

        if( !(rValue >>= xTemplate) )
            throw IllegalArgumentException();

        if( mpObj.is() )
            static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->setTableStyle(xTemplate);

        return true;
    }
    case OWN_ATTR_TABLETEMPLATE_FIRSTROW:
    case OWN_ATTR_TABLETEMPLATE_LASTROW:
    case OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN:
    case OWN_ATTR_TABLETEMPLATE_LASTCOLUMN:
    case OWN_ATTR_TABLETEMPLATE_BANDINGROWS:
    case OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS:
    {
        if( mpObj.is() )
        {
            TableStyleSettings aSettings( static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->getTableStyleSettings() );

            switch( pProperty->nWID )
            {
            case OWN_ATTR_TABLETEMPLATE_FIRSTROW:           rValue >>= aSettings.mbUseFirstRow; break;
            case OWN_ATTR_TABLETEMPLATE_LASTROW:            rValue >>= aSettings.mbUseLastRow; break;
            case OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN:        rValue >>= aSettings.mbUseFirstColumn; break;
            case OWN_ATTR_TABLETEMPLATE_LASTCOLUMN:         rValue >>= aSettings.mbUseLastColumn; break;
            case OWN_ATTR_TABLETEMPLATE_BANDINGROWS:        rValue >>= aSettings.mbUseRowBanding; break;
            case OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS:    rValue >>= aSettings.mbUseColumnBanding; break;
            }

            static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->setTableStyleSettings(aSettings);
        }

        return true;
    }
    default:
    {
        return SvxShape::setPropertyValueImpl( pProperty, rValue );
    }
    }
}

extern Graphic SvxGetGraphicForShape( SdrObject& rShape, bool bVector );

bool SvxTableShape::getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_OLEMODEL:
    {
        if( mpObj.is() )
        {
            rValue <<= static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->getTable();
        }
        return true;
    }
    case OWN_ATTR_TABLETEMPLATE:
    {
        if( mpObj.is() )
        {
            rValue <<= static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->getTableStyle();
        }
        return true;
    }
    case OWN_ATTR_BITMAP:
    {
        if( mpObj.is() )
        {
            Graphic aGraphic( SvxGetGraphicForShape( *mpObj.get(), true ) );
            rValue <<= aGraphic.GetXGraphic();
        }
        return true;
    }
    case OWN_ATTR_TABLETEMPLATE_FIRSTROW:
    case OWN_ATTR_TABLETEMPLATE_LASTROW:
    case OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN:
    case OWN_ATTR_TABLETEMPLATE_LASTCOLUMN:
    case OWN_ATTR_TABLETEMPLATE_BANDINGROWS:
    case OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS:
    {
        if( mpObj.is() )
        {
            TableStyleSettings aSettings( static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->getTableStyleSettings() );

            switch( pProperty->nWID )
            {
            case OWN_ATTR_TABLETEMPLATE_FIRSTROW:           rValue <<= aSettings.mbUseFirstRow; break;
            case OWN_ATTR_TABLETEMPLATE_LASTROW:            rValue <<= aSettings.mbUseLastRow; break;
            case OWN_ATTR_TABLETEMPLATE_FIRSTCOLUMN:        rValue <<= aSettings.mbUseFirstColumn; break;
            case OWN_ATTR_TABLETEMPLATE_LASTCOLUMN:         rValue <<= aSettings.mbUseLastColumn; break;
            case OWN_ATTR_TABLETEMPLATE_BANDINGROWS:        rValue <<= aSettings.mbUseRowBanding; break;
            case OWN_ATTR_TABLETEMPLATE_BANDINGCOULUMNS:    rValue <<= aSettings.mbUseColumnBanding; break;
            }
        }

        return true;
    }
    default:
    {
        return SvxShape::getPropertyValueImpl( pProperty, rValue );
    }
    }
}

void SvxTableShape::lock()
{
    SvxShape::lock();
    if( mpObj.is() )
        static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->uno_lock();
}

void SvxTableShape::unlock()
{
    if( mpObj.is() )
        static_cast< sdr::table::SdrTableObj* >( mpObj.get() )->uno_unlock();
    SvxShape::unlock();
}

