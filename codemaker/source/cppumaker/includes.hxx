/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_INCLUDES_HXX
#define INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_INCLUDES_HXX

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

    void addCppuUnotypeHxx() { m_includeCppuUnotypeHxx = true; }

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
    bool m_includeCppuUnotypeHxx;
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

#endif // INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_INCLUDES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
