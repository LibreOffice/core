/*************************************************************************
 *
 *  $RCSfile: tabletree.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 10:09:23 $
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

#ifndef _DBAUI_TABLETREE_HXX_
#define _DBAUI_TABLETREE_HXX_

#ifndef _DBAUI_MARKTREE_HXX_
#include "marktree.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//========================================================================
//= OTableTreeListBox
//========================================================================
class OTableTreeListBox : public OMarkableTreeListBox
{
protected:
    Image           m_aTableImage;
    Image           m_aViewImage;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

public:
    OTableTreeListBox( Window* pParent, WinBits nWinStyle = NULL );
    OTableTreeListBox( Window* pParent, const ResId& rResId );

    void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        { m_xORB = _rxORB; }

    /** fill the table list with the tables belonging to the connection described by the parameters
        <BR>
        The given URL is used to obtain a <type scope="com.sun.star.sdbc">Driver</type> object which can
        handle it. After this, the driver is asked (<type scope="com.sun.star.sdbcx">XDataDefinitionSupplier</type>)
        for a <type scope="com.sun.star.sdbcx">DatabaseDefinition</type> object for the connection.
        @return     the connection which was created for the given URL and the given params.
        @throws     <type scope="com::sun::star::sdbc">SQLException</type> if no connection could be created
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    UpdateTableList(
                        const ::rtl::OUString& _rConnectionURL,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > _rProperties)
                    throw(::com::sun::star::sdbc::SQLException);

    /** fill the table list with the tables and views determined by the two given containers
        @param      _rxConnMetaData meta data describing the connection where you got the object names from. Must not be NULL.
                                    Used to split the full qualified names into it's parts.
        @param      _rxTables       table container. May be NULL.
        @param      _rxView         view container. May be NULL.
    */
    virtual void    UpdateTableList(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxConnMetaData,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > _rxTables,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > _rxView);

protected:
    virtual void Command( const CommandEvent& rEvt );
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLETREE_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 28.09.00 13:15:45  fs
 ************************************************************************/

