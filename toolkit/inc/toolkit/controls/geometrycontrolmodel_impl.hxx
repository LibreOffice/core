/*************************************************************************
 *
 *  $RCSfile: geometrycontrolmodel_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2001-03-02 12:34:13 $
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
    return *getArrayHelper();
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
OGeometryControlModel_Base* OGeometryControlModel<CONTROLMODEL>::createClone_Impl(
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance)
{
    return new OGeometryControlModel<CONTROLMODEL>(_rxAggregateInstance);
}


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/24 14:57:30  mt
 *  model for dialog controls (weith pos/size)
 *
 *
 *  Revision 1.0 17.01.01 12:50:24  fs
 ************************************************************************/

