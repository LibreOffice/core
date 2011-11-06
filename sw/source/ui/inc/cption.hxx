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


#ifndef _CPTION_HXX
#define _CPTION_HXX

#include <svx/stddlg.hxx>

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <actctrl.hxx>


#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>

class SwFldMgr;
class SwView;

#include <wrtsh.hxx>
#include "optload.hxx"
#include "swlbox.hxx"


class SwCaptionDialog : public SvxStandardDialog
{
    class CategoryBox : public ComboBox
    {
    public:
        CategoryBox( Window* pParent, const ResId& rResId )
            : ComboBox( pParent, rResId )
        {}

        virtual long    PreNotify( NotifyEvent& rNEvt );
    };

    FixedText    aTextText;
    Edit         aTextEdit;
    FixedLine    aSettingsFL;
    FixedText    aCategoryText;
    CategoryBox  aCategoryBox;
    FixedText    aFormatText;
    ListBox      aFormatBox;
    //#i61007# order of captions
    FixedText    aNumberingSeparatorFT;
    Edit         aNumberingSeparatorED;
    FixedText    aSepText;
    Edit         aSepEdit;
    FixedText    aPosText;
    ListBox      aPosBox;
    OKButton     aOKButton;
    CancelButton aCancelButton;
    HelpButton   aHelpButton;
    PushButton   aAutoCaptionButton;
    PushButton   aOptionButton;

    String       sNone;

    SwCaptionPreview    aPrevWin;

    SwView       &rView; //Suchen per aktive ::com::sun::star::sdbcx::View vermeiden.
    SwFldMgr     *pMgr;      //Ptr um das include zu sparen
    SelectionType eType;

    String       sCharacterStyle;
    String       sObjectName;
    bool         bCopyAttributes;
    bool        bOrderNumberingFirst; //#i61007# order of captions

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xNameAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >         xNamed;

    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( OptionHdl, Button * );
    DECL_LINK( CaptionHdl, PushButton*);

    virtual void Apply();

    void    DrawSample();
    void    CheckButtonWidth();
    void    ApplyCaptionOrder(); //#i61007# order of captions

public:
     SwCaptionDialog( Window *pParent, SwView &rV );
    ~SwCaptionDialog();
};

#endif


