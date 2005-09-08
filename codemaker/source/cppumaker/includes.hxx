/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: includes.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:13:38 $
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

#ifndef INCLUDED_codemaker_source_cppumaker_includes_hxx
#define INCLUDED_codemaker_source_cppumaker_includes_hxx

#include "codemaker/dependencies.hxx"

#include "rtl/ustring.hxx"

class FileStream;
class TypeManager;

namespace codemaker { namespace cppumaker {

class Includes {
public:
    Includes(
        TypeManager const & manager,
        codemaker::Dependencies const & dependencies, bool hpp);

    ~Includes();

    void add(rtl::OString const & registryType);

    void addAny() { m_includeAny = true; }

    void addReference() { m_includeReference = true; }

    void addSequence() { m_includeSequence = true; }

    void addType() { m_includeType = true; }

    void addCppuMacrosHxx() { m_includeCppuMacrosHxx = true; }

    void addOslDoublecheckedlockingH()
    { m_includeOslDoublecheckedlockingH = true; }

    void addOslMutexHxx() { m_includeOslMutexHxx = true; }

    void addRtlStrbufHxx() { m_includeRtlStrbufHxx = true; }

    void addRtlStringH() { m_includeRtlStringH = true; }

    void addRtlTextencH() { m_includeRtlTextencH = true; }

    void addRtlUstrbufHxx() { m_includeRtlUstrbufHxx = true; }

    void addRtlUstringH() { m_includeRtlUstringH = true; }

    void addRtlUstringHxx() { m_includeRtlUstringHxx = true; }

    void addSalTypesH() { m_includeSalTypesH = true; }

    void addTypelibTypeclassH() { m_includeTypelibTypeclassH = true; }

    void addTypelibTypedescriptionH()
    { m_includeTypelibTypedescriptionH = true; }

    void dump(FileStream & out, rtl::OString const * companionHdl);

    static void dumpInclude(
        FileStream & out, rtl::OString const & registryType, bool hpp,
        rtl::OString const & suffix = rtl::OString());

private:
    Includes(Includes &); // not implemented
    void operator =(Includes); // not implemented;

    bool isInterfaceType(rtl::OString const & registryType) const;

    TypeManager const & m_manager;
    codemaker::Dependencies::Map m_map;
    bool m_hpp;
    bool m_includeAny;
    bool m_includeReference;
    bool m_includeSequence;
    bool m_includeType;
    bool m_includeCppuMacrosHxx;
    bool m_includeOslDoublecheckedlockingH;
    bool m_includeOslMutexHxx;
    bool m_includeRtlStrbufHxx;
    bool m_includeRtlStringH;
    bool m_includeRtlTextencH;
    bool m_includeRtlUstrbufHxx;
    bool m_includeRtlUstringH;
    bool m_includeRtlUstringHxx;
    bool m_includeSalTypesH;
    bool m_includeTypelibTypeclassH;
    bool m_includeTypelibTypedescriptionH;
};

} }

#endif
