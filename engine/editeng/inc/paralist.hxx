/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>
#include <sal/log.hxx>

#include <memory>
#include <vector>

#include <editeng/outliner.hxx>
#include <o3tl/safeint.hxx>
#include <tools/link.hxx>

typedef struct _xmlTextWriter* xmlTextWriterPtr;

class ParagraphList
{
public:
    static sal_Int32 GetParent( sal_Int32 nParagraphPos, const EditEngine& rEditEngine );
    static bool     HasChildren( sal_Int32 nParagraphPos, const EditEngine& rEditEngine  );
    static bool     HasHiddenChildren( sal_Int32 nParagraphPos, const EditEngine& rEditEngine );
    static bool     HasVisibleChildren( sal_Int32 nParagraphPos, const EditEngine& rEditEngine );
    static sal_Int32 GetChildCount( sal_Int32 nParagraphPos, const EditEngine& rEditEngine );

    void            Expand( sal_Int32 nParentParaPos, EditEngine& rEditEngine );
    void            Collapse( sal_Int32 nParentParaPos, EditEngine& rEditEngine );

    void            SetVisibleStateChangedHdl( const Link<sal_Int32,void>& rLink ) { aVisibleStateChangedHdl = rLink; }

private:

    Link<sal_Int32,void> aVisibleStateChangedHdl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

