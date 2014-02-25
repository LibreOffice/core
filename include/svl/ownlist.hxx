/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVL_OWNLIST_HXX
#define INCLUDED_SVL_OWNLIST_HXX

#include <svl/svldllapi.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <vector>

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
}}}


class SvCommand
/*  [Beschreibung]

    Enth"alt einen String, welcher das Kommando angibt und eine weiteren
    String, der das Argument des Kommandos bildet. W"urde solch ein
    Kommando "uber die Kommandozeile angegeben werden, s"ahe es wie folgt
    aus: Kommando = Argument.
*/
{
    OUString aCommand;
    OUString aArgument;
public:
                    SvCommand() {}
    SvCommand( const OUString & rCommand, const OUString & rArg )
                    {
                        aCommand = rCommand;
                        aArgument = rArg;
                    }
    const OUString & GetCommand() const { return aCommand; }
    const OUString & GetArgument() const { return aArgument; }
};

typedef ::std::vector< SvCommand > SvCommandList_impl;


class SVL_DLLPUBLIC SvCommandList
/*  [Beschreibung]

    Die Liste enth"alt Objekte vom Typ SvCommand. Wird ein Objekt
    eingef"ugt, dann wird es kopiert und das neue Objekt wird
    in die Liste gestellt.
*/
{
private:
    SvCommandList_impl  aCommandList;

public:
    SvCommand&      Append( const OUString & rCommand, const OUString & rArg );
    bool        AppendCommands( const OUString & rCmd, sal_Int32 * pEaten );

    bool FillFromSequence( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );
    void FillSequence( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& );

    size_t          size() const { return aCommandList.size(); }

    SvCommand       operator[]( size_t i) {
                        return aCommandList[ i ];
                    }

    void            clear() {
                        aCommandList.clear();
                    }
};

#endif // INCLUDED_SVL_OWNLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
