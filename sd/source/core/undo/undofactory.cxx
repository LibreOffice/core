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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <tools/debug.hxx>

#include "undo/undofactory.hxx"
#include "undo/undoobjects.hxx"

using namespace sd;

SdrUndoAction* UndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new UndoRemoveObject( rObject, bOrdNumDirect );
}

SdrUndoAction* UndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new UndoDeleteObject( rObject, bOrdNumDirect );
}

SdrUndoAction* UndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return new UndoObjectSetText( rNewObj, nText );
}

SdrUndoAction* UndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return new UndoReplaceObject( rOldObject, rNewObject, bOrdNumDirect );
}

SdrUndoAction* UndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return new UndoGeoObject( rObject );
}

SdrUndoAction* UndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return new UndoAttrObject( rObject, bStyleSheet1, bSaveText );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
