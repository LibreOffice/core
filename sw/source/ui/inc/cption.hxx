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
#ifndef _CPTION_HXX
#define _CPTION_HXX

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/group.hxx>

#include <vcl/button.hxx>
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
