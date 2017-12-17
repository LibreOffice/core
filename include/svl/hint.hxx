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
#ifndef INCLUDED_SVL_HINT_HXX
#define INCLUDED_SVL_HINT_HXX

#include <sal/types.h>
#include <svl/svldllapi.h>

enum class SfxHintId {
    NONE,
    Dying,
    NameChanged,
    TitleChanged,
    DataChanged,
    DocChanged,
    UpdateDone,
    Deinitializing,
    ModeChanged,
    ColorsChanged,
    LanguageChanged,
    RedlineChanged,
    DocumentRepair,

// VCL text hints
    TextParaInserted,
    TextParaRemoved,
    TextParaContentChanged,
    TextHeightChanged,
    TextFormatPara,
    TextFormatted,
    TextModified,
    TextBlockNotificationStart,
    TextBlockNotificationEnd,
    TextInputStart,
    TextInputEnd,
    TextViewScrolled,
    TextViewSelectionChanged,
    TextViewCaretChanged,

// BASIC hints
    BasicDying,
    BasicDataWanted,
    BasicDataChanged,
    BasicConverted,
    BasicInfoWanted,
    BasicObjectChanged,
    BasicStart,
    BasicStop,

// SVX edit source
    EditSourceParasMoved,
    EditSourceSelectionChanged,

// SC hints
    ScDataChanged,
    ScTableOpDirty,
    ScCalcAll,
    ScReference,
    ScDrawLayerNew,
    ScDbAreasChanged,
    ScAreasChanged,
    ScTablesChanged,
    ScDrawChanged,
    ScDocNameChanged,
    ScAreaLinksChanged,
    ScShowRangeFinder,
    ScDocSaved,
    ScForceSetTab,
    ScNavigatorUpdateAll,
    ScAnyDataChanged,
    ScPrintOptions,
    ScRefModeChanged,
    ScKillEditView,
    ScKillEditViewNoPaint,
    ScHiddenRowsChanged,
    ScSelectionChanged,
    ScClearCache,

// SC accessibility hints
    ScAccTableChanged,
    ScAccCursorChanged,
    ScAccVisAreaChanged,
    ScAccEnterEditMode,
    ScAccLeaveEditMode,
    ScAccMakeDrawLayer,
    ScAccWindowResized,


// SFX stylesheet
    StyleSheetCreated,  // new
    StyleSheetModified,  // changed
    StyleSheetChanged,  // erased and re-created (replaced)
    StyleSheetErased,  // erased
    StyleSheetInDestruction,  // in the process of being destructed

// STARMATH
    MathFormatChanged,

// SW
    SwDrawViewsCreated,
    SwSplitNodeOperation,
};

class SVL_DLLPUBLIC SfxHint
{
private:
    SfxHintId mnId;
public:
    SfxHint() : mnId(SfxHintId::NONE) {}
    explicit SfxHint( SfxHintId nId ) : mnId(nId) {}
    virtual ~SfxHint() COVERITY_NOEXCEPT_FALSE;
    SfxHintId GetId() const { return mnId; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
