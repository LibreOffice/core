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



#ifndef DBACCESS_ADMINCONTROLS_HXX
#define DBACCESS_ADMINCONTROLS_HXX

#include "adminpages.hxx"

#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

#include <svtools/dialogcontrolling.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= MySQLNativeSettings
    //====================================================================
    class MySQLNativeSettings : public Control
    {
    private:
        FixedText           m_aDatabaseNameLabel;
        Edit                m_aDatabaseName;
        RadioButton         m_aHostPortRadio;
        RadioButton         m_aSocketRadio;
        RadioButton         m_aNamedPipeRadio;
        FixedText           m_aHostNameLabel;
        Edit                m_aHostName;
        FixedText           m_aPortLabel;
        NumericField        m_aPort;
        FixedText           m_aDefaultPort;
        Edit                m_aSocket;
        Edit                m_aNamedPipe;

        ::svt::ControlDependencyManager
                            m_aControlDependencies;

    public:
        MySQLNativeSettings( Window& _rParent, const Link& _rControlModificationLink );
        ~MySQLNativeSettings();

        void fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList );
        void fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList );

        sal_Bool FillItemSet( SfxItemSet& _rCoreAttrs );
        void implInitControls( const SfxItemSet& _rSet );

        bool canAdvance() const;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ADMINCONTROLS_HXX
