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



#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/edit.hxx>

#include <set>
#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= NewDataTypeDialog
    //====================================================================
    class NewDataTypeDialog : public ModalDialog
    {
    private:
        FixedText       m_aLabel;
        Edit            m_aName;
        OKButton        m_aOK;
        CancelButton    m_aCancel;

        ::std::set< ::rtl::OUString >
                        m_aProhibitedNames;

    public:
        NewDataTypeDialog( Window* _pParent, const ::rtl::OUString& _rNameBase, const ::std::vector< ::rtl::OUString >& _rProhibitedNames );

        inline String   GetName() const { return m_aName.GetText(); }

    private:
        DECL_LINK( OnNameModified, void* );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

