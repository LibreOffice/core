/*************************************************************************
 *
 *  $RCSfile: MutableAttrList.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:53:37 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SvXMLAttributeList *XMLMutableAttributeList::GetMutableAttrList()
{
    if( !m_pMutableAttrList )
    {
        m_pMutableAttrList = new SvXMLAttributeList( m_xAttrList );
        m_xAttrList = m_pMutableAttrList;
    }

    return m_pMutableAttrList;
}

XMLMutableAttributeList::XMLMutableAttributeList() :
    m_pMutableAttrList( new SvXMLAttributeList )
{
    m_xAttrList = m_pMutableAttrList;
}

XMLMutableAttributeList::XMLMutableAttributeList( const Reference<
        XAttributeList> & rAttrList, sal_Bool bClone ) :
    m_xAttrList( rAttrList ),
    m_pMutableAttrList( 0 )
{
    if( bClone )
        GetMutableAttrList();
}


XMLMutableAttributeList::~XMLMutableAttributeList()
{
    m_xAttrList = 0;
}


// XUnoTunnel & co
const Sequence< sal_Int8 > & XMLMutableAttributeList::getUnoTunnelId() throw()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        Guard< Mutex > aGuard( Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

XMLMutableAttributeList* XMLMutableAttributeList::getImplementation(
        Reference< XInterface > xInt ) throw()
{
    Reference< XUnoTunnel > xUT( xInt, UNO_QUERY );
    if( xUT.is() )
        return (XMLMutableAttributeList*)xUT->getSomething(
                                XMLMutableAttributeList::getUnoTunnelId() );
    else
        return NULL;
}

// XUnoTunnel
sal_Int64 SAL_CALL XMLMutableAttributeList::getSomething(
        const Sequence< sal_Int8 >& rId )
    throw( RuntimeException )
{
    if( rId.getLength() == 16 &&
        0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

sal_Int16 SAL_CALL XMLMutableAttributeList::getLength(void)
        throw( RuntimeException )
{
    return m_xAttrList->getLength();
}


OUString SAL_CALL XMLMutableAttributeList::getNameByIndex(sal_Int16 i)
        throw( RuntimeException )
{
    return m_xAttrList->getNameByIndex( i );
}


OUString SAL_CALL XMLMutableAttributeList::getTypeByIndex(sal_Int16 i)
        throw( RuntimeException )
{
    return m_xAttrList->getTypeByIndex( i );
}

OUString SAL_CALL  XMLMutableAttributeList::getValueByIndex(sal_Int16 i)
    throw( RuntimeException )
{
    return m_xAttrList->getValueByIndex( i );
}

OUString SAL_CALL XMLMutableAttributeList::getTypeByName(
        const OUString& rName )
        throw( RuntimeException )
{
    return m_xAttrList->getTypeByName( rName );
}

OUString SAL_CALL XMLMutableAttributeList::getValueByName(
        const OUString& rName)
        throw( RuntimeException )
{
    return m_xAttrList->getValueByName( rName );
}


Reference< XCloneable > XMLMutableAttributeList::createClone()
        throw( RuntimeException )
{
    // A cloned list will be a read only list!
    Reference< XCloneable >  r = new SvXMLAttributeList( m_xAttrList );
    return r;
}

void XMLMutableAttributeList::SetValueByIndex( sal_Int16 i,
                                               const ::rtl::OUString& rValue )
{
    GetMutableAttrList()->SetValueByIndex( i, rValue );
}

void XMLMutableAttributeList::AddAttribute( const OUString &rName ,
                                            const OUString &rValue )
{
    GetMutableAttrList()->AddAttribute( rName, rValue );
}

void XMLMutableAttributeList::RemoveAttributeByIndex( sal_Int16 i )
{
    GetMutableAttrList()->RemoveAttributeByIndex( i );
}

void XMLMutableAttributeList::RenameAttributeByIndex( sal_Int16 i,
                                                      const OUString& rNewName )
{
    GetMutableAttrList()->RenameAttributeByIndex( i, rNewName );
}

void XMLMutableAttributeList::AppendAttributeList(
        const Reference< ::com::sun::star::xml::sax::XAttributeList >& r )
{
    GetMutableAttrList()->AppendAttributeList( r );
}

sal_Int16 XMLMutableAttributeList::GetIndexByName( const OUString& rName ) const
{
    sal_Int16 nIndex = -1;
    if( m_pMutableAttrList )
    {
        nIndex = m_pMutableAttrList->GetIndexByName( rName );
    }
    else
    {
        sal_Int16 nCount = m_xAttrList->getLength();
        for( sal_Int16 i=0; nIndex!=-1 && i<nCount ; ++i )
        {
            if( m_xAttrList->getValueByIndex(i) == rName )
                nIndex = i;
        }
    }
    return nIndex;
}
