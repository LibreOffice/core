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



#ifndef _BASDOC_HXX
#define _BASDOC_HXX

#include <svx/ifaceids.hxx>
#include <iderid.hxx>
#include <sfx2/objsh.hxx>

class SfxPrinter;

class BasicDocShell: public SfxObjectShell
{
    SfxPrinter*         pPrinter;

protected:
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          sal_uInt16 nAspect = ASPECT_CONTENT );
    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nVersion,
                               sal_Bool bTemplate = sal_False ) const;

public:
                        using SotObject::GetInterface;
                        SFX_DECL_OBJECTFACTORY();
                        SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_DOCSH )
                        BasicDocShell();
                        ~BasicDocShell();

    SfxPrinter*         GetPrinter( sal_Bool bCreate );
    void                SetPrinter( SfxPrinter* pPrinter );
};

#endif  // _BASDOC_HXX
