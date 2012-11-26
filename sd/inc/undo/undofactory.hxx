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



#ifndef _SD_UNDOFACTORY_HXX
#define _SD_UNDOFACTORY_HXX

#include <svx/svdundo.hxx>

namespace sd
{

class UndoFactory : public SdrUndoFactory
{
public:
    virtual SdrUndoAction* CreateUndoRemoveObject( SdrObject& rObject );
    virtual SdrUndoAction* CreateUndoDeleteObject( SdrObject& rObject );
    virtual SdrUndoAction* CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText );
    virtual SdrUndoAction* CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject );
    virtual SdrUndoAction* CreateUndoGeoObject( SdrObject& rObject );
    virtual SdrUndoAction* CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1 = false, bool bSaveText = false );
};


}

#endif     // _SD_UNDOMANAGER_HXX
