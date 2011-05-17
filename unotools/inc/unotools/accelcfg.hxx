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
#ifndef INCLUDED_unotools_ACCELCFG_HXX
#define INCLUDED_unotools_ACCELCFG_HXX

#include <com/sun/star/awt/KeyEvent.hpp>
#include <unotools/options.hxx>

#include <rtl/ustring.hxx>
#include <tools/stream.hxx>

struct SvtAcceleratorConfigItem
{
    sal_uInt16      nCode;
    sal_uInt16      nModifier;
    ::rtl::OUString aCommand;
};

#include <list>
typedef ::std::list < SvtAcceleratorConfigItem > SvtAcceleratorItemList;

class SvStream;
class KeyEvent;
class String;
class SvtAcceleratorConfig_Impl;

class SvtAcceleratorConfiguration: public utl::detail::Options
{
    SvtAcceleratorConfig_Impl*    pImp;

private:

public:
                    // get the global accelerators
                    SvtAcceleratorConfiguration();

                    // get special accelerators
    static SvtAcceleratorConfiguration* CreateFromStream( SvStream& rStream );
    static String   GetStreamName();
    static SvStream* GetDefaultStream( StreamMode );

                    virtual ~SvtAcceleratorConfiguration();

                    // save the configuration to a stream, f.e. into a document
    bool            Commit( SvStream& rStream );

                    // returns the configured URL for a KeyEvent
    ::rtl::OUString GetCommand( const ::com::sun::star::awt::KeyEvent& rKeyEvent );

                    // returns the whole configuration
    const SvtAcceleratorItemList& GetItems();

                    // sets a single configuration item
    void            SetCommand( const SvtAcceleratorConfigItem& rItem );

                    // sets several or all configuration items
                    // if bClear=sal_False, all items not in the sequence remain unchanged
    void            SetItems( const SvtAcceleratorItemList& rItems, bool bClear );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
