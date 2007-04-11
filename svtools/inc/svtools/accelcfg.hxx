/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accelcfg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:10:45 $
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
#ifndef INCLUDED_SVTOOLS_ACCELCFG_HXX
#define INCLUDED_SVTOOLS_ACCELCFG_HXX

#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

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

class SvtAcceleratorConfiguration: public svt::detail::Options
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
                    // if bClear=FALSE, all items not in the sequence remain unchanged
    void            SetItems( const SvtAcceleratorItemList& rItems, bool bClear );
};

#endif
