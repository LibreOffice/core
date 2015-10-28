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

#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONCREATEDIALOG_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONCREATEDIALOG_HXX

#include "CustomAnimationPreset.hxx"
#include <vcl/tabdlg.hxx>

enum class PathKind { NONE, CURVE, POLYGON, FREEFORM };

class TabControl;
class OKButton;
class CancelButton;
class HelpButton;

namespace sd {

class CustomAnimationCreateTabPage;
class CustomAnimationPane;

class CustomAnimationCreateDialog : public TabDialog
{
    friend class CustomAnimationCreateTabPage;
public:
    CustomAnimationCreateDialog( vcl::Window* pParent, CustomAnimationPane* pPane, const std::vector< css::uno::Any >& rTargets, bool bHasText, const OUString& rsPresetId, double fDuration );
    virtual ~CustomAnimationCreateDialog();
    virtual void dispose() override;

    PathKind getCreatePathKind() const;
    CustomAnimationPresetPtr getSelectedPreset() const;
    double getSelectedDuration() const;

private:
    CustomAnimationCreateTabPage* getCurrentPage() const;
    void preview( const CustomAnimationPresetPtr& pPreset ) const;
    void setPosition();
    void storePosition();

    DECL_LINK_TYPED(implActivatePagekHdl, TabControl*, void);
    DECL_LINK_TYPED(implDeactivatePagekHdl, TabControl*, bool);

private:
    VclPtr<CustomAnimationPane> mpPane;
    const std::vector< css::uno::Any >& mrTargets;

    double mfDuration;
    bool mbIsPreview;

    VclPtr<TabControl> mpTabControl;

    VclPtr<CustomAnimationCreateTabPage> mpTabPages[5];
    sal_uInt16 mnEntranceId, mnEmphasisId, mnExitId, mnMPathId, mnMiscId;
};

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONCREATEDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
