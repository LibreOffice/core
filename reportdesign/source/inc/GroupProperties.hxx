#ifndef RPT_SHARED_GROUPS_PROPERTIES_HXX
#define RPT_SHARED_GROUPS_PROPERTIES_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GroupProperties.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:28 $
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

namespace rptshared
{
    /** struct for the group properties
    */
    struct GroupProperties
    {
        ::sal_Int32                                                                 m_nGroupInterval;
        ::rtl::OUString                                                             m_sExpression;
        ::sal_Int16                                                                 m_nGroupOn;
        ::sal_Int16                                                                 m_nKeepTogether;
        ::sal_Bool                                                                  m_eSortAscending;
        ::sal_Bool                                                                  m_bStartNewColumn;
        ::sal_Bool                                                                  m_bResetPageNumber;

        GroupProperties()
        :m_nGroupInterval(1)
        ,m_nGroupOn(0)
        ,m_nKeepTogether(0)
        ,m_eSortAscending(sal_True)
        ,m_bStartNewColumn(sal_False)
        ,m_bResetPageNumber(sal_False)
        {}
    };
}
#endif //RPT_SHARED_GROUPS_PROPERTIES_HXX
