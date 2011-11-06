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


#ifndef SVT_QUERYFOLDER_HXX
#define SVT_QUERYFOLDER_HXX

#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/edit.hxx>
#include <vcl/dialog.hxx>

//-------------------------------------------------------------------------
namespace svtools {

//-------------------------------------------------------------------------
// QueryFolderNameDialog
//-------------------------------------------------------------------------

class QueryFolderNameDialog : public ModalDialog
{
private:
    FixedText       aNameText;
    Edit            aNameEdit;
    FixedLine       aNameLine;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;

    DECL_LINK( OKHdl, Button * );
    DECL_LINK( NameHdl, Edit * );

public:
                    QueryFolderNameDialog(  Window* _pParent,
                                            const String& rTitle,
                                            const String& rDefaultText,
                                            String* pGroupName = NULL );
    String          GetName() const { return aNameEdit.GetText(); }
};

}
#endif // SVT_QUERYFOLDER_HXX

