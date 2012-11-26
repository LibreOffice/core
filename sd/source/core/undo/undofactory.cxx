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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <tools/debug.hxx>

#include "undo/undofactory.hxx"
#include "undo/undoobjects.hxx"

using namespace sd;

SdrUndoAction* UndoFactory::CreateUndoRemoveObject( SdrObject& rObject )
{
    return new UndoRemoveObject( rObject );
}

SdrUndoAction* UndoFactory::CreateUndoDeleteObject( SdrObject& rObject )
{
    return new UndoDeleteObject( rObject );
}

SdrUndoAction* UndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return new UndoObjectSetText( rNewObj, nText );
}

SdrUndoAction* UndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject )
{
    return new UndoReplaceObject( rOldObject, rNewObject );
}

SdrUndoAction* UndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return new UndoGeoObject( rObject );
}

SdrUndoAction* UndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return new UndoAttrObject( rObject, bStyleSheet1, bSaveText );
}
