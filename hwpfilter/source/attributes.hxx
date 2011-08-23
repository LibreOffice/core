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


#ifndef _CONFIGMGR_MISC_ATTRIBUTES_HXX_
#define _CONFIGMGR_MISC_ATTRIBUTES_HXX_

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <cppuhelper/implbase1.hxx>

/*----------------------------------------
*
* 	Attributlist implementation
*
*----------------------------------------*/

using namespace ::cppu;
using namespace ::rtl;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

struct AttributeListImpl_impl;
class AttributeListImpl : public WeakImplHelper1< XAttributeList >
{
protected:
    ~AttributeListImpl();

public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );
    
public:
    virtual sal_Int16 SAL_CALL getLength(void) throw (RuntimeException);
    virtual OUString  SAL_CALL getNameByIndex(sal_Int16 i) throw (RuntimeException);
    virtual OUString  SAL_CALL getTypeByIndex(sal_Int16 i) throw (RuntimeException);
    virtual OUString  SAL_CALL getTypeByName(const OUString& aName) throw (RuntimeException);
    virtual OUString  SAL_CALL getValueByIndex(sal_Int16 i) throw (RuntimeException);
    virtual OUString  SAL_CALL getValueByName(const OUString& aName) throw (RuntimeException);

public:
    void addAttribute( const OUString &sName , const OUString &sType , const OUString &sValue );
    void clear();

private:
    struct AttributeListImpl_impl *m_pImpl;
};

#endif // _CONFIGMGR_MISC_ATTRIBUTES_HXX_


