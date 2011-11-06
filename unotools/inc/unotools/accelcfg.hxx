/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
