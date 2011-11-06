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



#ifndef TEXTCONNECTIONSETTINGS_HXX
#define TEXTCONNECTIONSETTINGS_HXX

#include "propertystorage.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>

#include <memory>

class SfxItemSet;
//........................................................................
namespace dbaui
{
//........................................................................

    class OTextConnectionHelper;

    //====================================================================
    //= TextConnectionSettingsDialog
    //====================================================================
    class TextConnectionSettingsDialog : public ModalDialog
    {
    public:
        TextConnectionSettingsDialog( Window* _pParent, SfxItemSet& _rItems );
        ~TextConnectionSettingsDialog();

        /** initializes a set of PropertyStorage instaces, which are bound to
            the text-connection relevant items in our item sets
        */
        static void bindItemStorages( SfxItemSet& _rSet, PropertyValues& _rValues );

        virtual short   Execute();

    private:
        ::std::auto_ptr< OTextConnectionHelper >    m_pTextConnectionHelper;
        OKButton                                    m_aOK;
        CancelButton                                m_aCancel;
        SfxItemSet&                                 m_rItems;

    private:
        DECL_LINK( OnOK, PushButton* );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // TEXTCONNECTIONSETTINGS_HXX
