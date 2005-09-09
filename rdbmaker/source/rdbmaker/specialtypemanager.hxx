/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: specialtypemanager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:07:00 $
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

#ifndef _SPECIALTYPEMANAGER_HXX_
#define _SPECIALTYPEMANAGER_HXX_

#ifndef _CODEMAKER_REGISTRY_HXX_
#include    <codemaker/registry.hxx>
#endif

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include    <codemaker/typemanager.hxx>
#endif


struct SpecialTypeManagerImpl
{
    T2TypeClassMap  m_t2TypeClass;
};

class SpecialTypeManager : public TypeManager
{
public:
    SpecialTypeManager();
    ~SpecialTypeManager();

    SpecialTypeManager( const SpecialTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {
        acquire();
    }

    sal_Bool init(const ::rtl::OString& registryName);

    sal_Bool    isValidType(const ::rtl::OString& name)
        { return sal_True; }
    TypeReader  getTypeReader(const ::rtl::OString& name);
    RTTypeClass getTypeClass(const ::rtl::OString& name);

    sal_Int32 getSize() { return m_pImpl->m_t2TypeClass.size(); }

protected:
    void acquire();
    void release();

protected:
    SpecialTypeManagerImpl* m_pImpl;
};

#endif // _CODEMAKER_TYPEMANAGER_HXX_
