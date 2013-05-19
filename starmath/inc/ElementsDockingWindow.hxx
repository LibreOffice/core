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
#ifndef _ELEMENTSDOCKINGWINDOW_HXX_
#define _ELEMENTSDOCKINGWINDOW_HXX_

#include <sfx2/ctrlitem.hxx>
#include <sfx2/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/graph.hxx>

#include <svx/dlgctrl.hxx>
#include <document.hxx>

class SmElement
{
    SmNode* mpNode;
    OUString maText;
public:
    SmElement(SmNode* pNode, OUString aText);
    virtual ~SmElement();
    SmNode* getNode();

    OUString getText() { return maText; }

    virtual bool isSeparator()
    {
        return false;
    }

    Point mBoxLocation;
    Size  mBoxSize;
};

class SmElementSeparator : public SmElement
{
public:
    SmElementSeparator();

    virtual bool isSeparator()
    {
        return true;
    }
};

class SmElementsControl : public Control
{

    static const sal_uInt16 aUnaryBinaryOperatorsList[];
    static const sal_uInt16 aRelationsList[];
    static const sal_uInt16 aSetOperations[];
    static const sal_uInt16 aFunctions[];
    static const sal_uInt16 aOperators[];
    static const sal_uInt16 aAttributes[];
    static const sal_uInt16 aBrackets[];
    static const sal_uInt16 aFormats[];
    static const sal_uInt16 aOthers[];

    Link aSelectHdlLink;

    virtual void Paint(const Rectangle&);
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void MouseMove( const MouseEvent& rMEvt );

    SmDocShell* mpDocShell;
    SmFormat    maFormat;
    sal_uInt16  maCurrentSetId;
    SmElement*  mpCurrentElement;

    std::vector<SmElement*> maElementList;
    Size                    maMaxElementDimensions;

    void addElement(OUString aElementVisual, OUString aElementSource);

    void addElements(const sal_uInt16 aElementsArray[], sal_uInt16 size);

    void addSeparator();

    void build();

public:
    SmElementsControl(Window *pParent, const ResId& rResId);

    void setElementSetId(sal_uInt16 aSetId);

    void SetSelectHdl(const Link& rLink)   { aSelectHdlLink = rLink; }
};

class SmElementsDockingWindow : public SfxDockingWindow
{
    static const sal_uInt16 aCategories[];

    SmElementsControl   maElementsControl;
    ListBox             maElementListBox;

    virtual void Resize();
    SmViewShell* GetView();

    DECL_LINK(SelectClickHdl, SmElement*);
    DECL_LINK(ElementSelectedHandle, ListBox*);

public:

    SmElementsDockingWindow( SfxBindings* pBindings,
                             SfxChildWindow* pChildWindow,
                             Window* pParent );
    ~SmElementsDockingWindow();
};

class SmElementsDockingWindowWrapper : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(SmElementsDockingWindowWrapper);

protected:
    SmElementsDockingWindowWrapper( Window* pParentWindow,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo );
    virtual ~SmElementsDockingWindowWrapper();
};


#endif // _SYMBOLDOCKINGWINDOW_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
