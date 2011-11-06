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



#ifndef _IDERDLL2_HXX
#define _IDERDLL2_HXX

class StarBASIC;
class SvxSearchItem;
class Accelerator;

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>

#include <bastypes.hxx>
#include <bastype2.hxx>

#define INVPOSITION 0x7fff

class BasicIDEData
{
private:
    Accelerator*    pAccelerator;
    SvxSearchItem*  pSearchItem;

    LibInfos        aLibInfos;

    BasicEntryDescriptor    m_aLastEntryDesc;

    Point           aObjCatPos;
    Size            aObjCatSize;

    String          aAddLibPath;
    String          aAddLibFilter;

    sal_uInt16          nBasicDialogCount;

    sal_Bool            OLD_bRelMacroRecording;
    sal_Bool            bChoosingMacro;
    sal_Bool            bShellInCriticalSection;

protected:
    DECL_LINK( GlobalBasicBreakHdl, StarBASIC * );

public:
                            BasicIDEData();
                            ~BasicIDEData();

    LibInfos&               GetLibInfos() { return aLibInfos; }

    BasicEntryDescriptor&   GetLastEntryDescriptor()    { return m_aLastEntryDesc; }
    void                    SetLastEntryDescriptor( BasicEntryDescriptor& rDesc ) { m_aLastEntryDesc = rDesc; }

    sal_Bool&                   ChoosingMacro()             { return bChoosingMacro; }
    sal_Bool&                   ShellInCriticalSection()    { return bShellInCriticalSection; }

    sal_uInt16                  GetBasicDialogCount() const { return nBasicDialogCount; }
    void                    IncBasicDialogCount()       { nBasicDialogCount++; }
    void                    DecBasicDialogCount()       { nBasicDialogCount--; }

    SvxSearchItem&          GetSearchItem() const;
    void                    SetSearchItem( const SvxSearchItem& rItem );

    void                    SetObjectCatalogPos( const Point& rPnt )
                                { aObjCatPos = rPnt; }
    const Point&            GetObjectCatalogPos() const
                                { return aObjCatPos; }

    void                    SetObjectCatalogSize( const Size& rSize )
                                { aObjCatSize = rSize; }
    const Size&             GetObjectCatalogSize() const
                                { return aObjCatSize; }

    const String&           GetAddLibPath() const   { return aAddLibPath; }
    void                    SetAddLibPath( const String& rPath ) { aAddLibPath = rPath; }

    const String&           GetAddLibFilter() const { return aAddLibFilter; }
    void                    SetAddLibFilter( const String& rFilter )  { aAddLibFilter = rFilter; }
};


#endif //_IDERDLL2_HXX
