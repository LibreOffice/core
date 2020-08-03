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

#pragma once

#include <sal/config.h>
#include <sal/types.h>
#include <memory>

class SdPage;
class SdrUndoAction;

namespace com::sun::star::office {
    class XAnnotation;
}

namespace com::sun::star::uno { template <typename > class Reference; }

class SfxViewShell;

namespace sd {

enum class CommentNotificationType { Add, Modify, Remove };

void createAnnotation( css::uno::Reference< css::office::XAnnotation >& xAnnotation, SdPage* pPage );

std::unique_ptr<SdrUndoAction> CreateUndoInsertOrRemoveAnnotation( const css::uno::Reference< css::office::XAnnotation >& xAnnotation, bool bInsert );

void CreateChangeUndo(const css::uno::Reference< css::office::XAnnotation >& xAnnotation);

sal_uInt32 getAnnotationId(const css::uno::Reference <css::office::XAnnotation>& xAnnotation);

const SdPage* getAnnotationPage(const css::uno::Reference<css::office::XAnnotation>& xAnnotation);

void LOKCommentNotify(CommentNotificationType nType, const SfxViewShell* pViewShell,
        css::uno::Reference<css::office::XAnnotation> const & rxAnnotation);

void LOKCommentNotifyAll(CommentNotificationType nType,
        css::uno::Reference<css::office::XAnnotation> const & rxAnnotation);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
