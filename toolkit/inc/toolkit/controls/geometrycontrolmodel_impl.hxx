/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: geometrycontrolmodel_impl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:49:20 $
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

// no include protection. This is included from within geometrycontrolmodel.hxx only

//====================================================================
//= OGeometryControlModel
//====================================================================
//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel<CONTROLMODEL>::OGeometryControlModel()
    :OGeometryControlModel_Base(new CONTROLMODEL)
{
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel<CONTROLMODEL>::OGeometryControlModel(::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance)
    :OGeometryControlModel_Base(_rxAggregateInstance)
{
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
::cppu::IPropertyArrayHelper& SAL_CALL OGeometryControlModel<CONTROLMODEL>::getInfoHelper()
{
    return *this->getArrayHelper();
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
void OGeometryControlModel<CONTROLMODEL>::fillProperties(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rProps, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rAggregateProps) const
{
    // our own properties
    OPropertyContainer::describeProperties(_rProps);
    // the aggregate properties
    if (m_xAggregateSet.is())
        _rAggregateProps = m_xAggregateSet->getPropertySetInfo()->getProperties();
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL OGeometryControlModel<CONTROLMODEL>::getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException)
{
    static ::cppu::OImplementationId * pId = NULL;
    if ( !pId )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static ::cppu::OImplementationId s_aId;
            pId = &s_aId;
        }
    }
    return pId->getImplementationId();
}

//--------------------------------------------------------------------
template <class CONTROLMODEL>
OGeometryControlModel_Base* OGeometryControlModel<CONTROLMODEL>::createClone_Impl(
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance)
{
    return new OGeometryControlModel<CONTROLMODEL>(_rxAggregateInstance);
}


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4.122.1  2005/09/05 16:57:39  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.4  2004/07/30 15:33:34  kz
 *  INTEGRATION: CWS gcc340fixes01 (1.3.268); FILE MERGED
 *  2004/07/13 16:56:04 hr 1.3.268.1: #i31439#: fix template resolution
 *
 *  Revision 1.3.268.1  2004/07/13 16:56:04  hr
 *  #i31439#: fix template resolution
 *
 *  Revision 1.3  2001/09/05 06:40:48  fs
 *  #88891# override the XTypeProvider methods
 *
 *  Revision 1.2  2001/03/02 12:34:13  tbe
 *  clone geometry control model
 *
 *  Revision 1.1  2001/01/24 14:57:30  mt
 *  model for dialog controls (weith pos/size)
 *
 *
 *  Revision 1.0 17.01.01 12:50:24  fs
 ************************************************************************/

