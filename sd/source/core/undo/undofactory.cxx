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

#include <undo/undofactory.hxx>
#include <undo/undoobjects.hxx>

using namespace sd;

std::unique_ptr<SdrUndoAction> UndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<UndoRemoveObject>( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> UndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<UndoDeleteObject>( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> UndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return std::make_unique<UndoObjectSetText>( rNewObj, nText );
}

std::unique_ptr<SdrUndoAction> UndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return std::make_unique<UndoReplaceObject>( rOldObject, rNewObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> UndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return std::make_unique<UndoGeoObject>( rObject );
}

std::unique_ptr<SdrUndoAction> UndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return std::make_unique<UndoAttrObject>( rObject, bStyleSheet1, bSaveText );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
