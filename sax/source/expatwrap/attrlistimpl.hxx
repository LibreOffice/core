/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attrlistimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 12:03:33 $
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
#include <cppuhelper/implbase2.hxx>

namespace sax_expatwrap
{

struct AttributeListImpl_impl;

class AttributeListImpl :
    public WeakImplHelper2< XAttributeList, XCloneable >
{
public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );
    ~AttributeListImpl();

public:
    virtual sal_Int16 SAL_CALL getLength(void) throw(RuntimeException);
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i) throw(RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i) throw(RuntimeException);
    virtual OUString SAL_CALL getTypeByName(const OUString& aName) throw(RuntimeException);
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i) throw(RuntimeException);
    virtual OUString SAL_CALL getValueByName(const OUString& aName) throw( RuntimeException);

public:
    virtual Reference< XCloneable >  SAL_CALL createClone() throw(RuntimeException);

public:
    void addAttribute( const OUString &sName , const OUString &sType , const OUString &sValue );
    void clear();
    void removeAttribute( const OUString &sName );
    void setAttributeList( const Reference<  XAttributeList > & );

private:
    struct AttributeListImpl_impl *m_pImpl;
};

}
