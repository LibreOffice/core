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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NAVICFG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NAVICFG_HXX

#include <unotools/configitem.hxx>

enum class RegionMode;
enum class ContentTypeId;

class SwNavigationConfig : public utl::ConfigItem
{
    ContentTypeId  nRootType;      //RootType
    sal_Int32      nSelectedPos;   //SelectedPosition
    sal_Int32      nOutlineLevel;  //OutlineLevel
    RegionMode     nRegionMode;    //InsertMode
    sal_Int32      nActiveBlock;   //ActiveBlock//Expand/CollapsState
    bool           bIsSmall;       //ShowListBox
    bool           bIsGlobalActive; //GlobalDocMode// global view for GlobalDoc valid?

    static css::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwNavigationConfig();
    virtual ~SwNavigationConfig();

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    ContentTypeId GetRootType()const {return nRootType;}
    void        SetRootType(ContentTypeId nSet){
                        if(nRootType != nSet)
                        {
                            SetModified();
                            nRootType = nSet;
                        }
                    }

    sal_Int32   GetOutlineLevel()const {return nOutlineLevel;}
    void        SetOutlineLevel(sal_Int32 nSet){
                        if(nOutlineLevel != nSet)
                        {
                            SetModified();
                            nOutlineLevel = nSet;
                        }
                    }

    RegionMode  GetRegionMode()const {return nRegionMode;}
    void        SetRegionMode(RegionMode nSet){
                    if(nRegionMode != nSet)
                    {
                        SetModified();
                        nRegionMode = nSet;
                    }
                }

    sal_Int32   GetActiveBlock()const {return nActiveBlock;}
    void        SetActiveBlock(sal_Int32 nSet){
                        if(nActiveBlock != nSet)
                        {
                            SetModified();
                            nActiveBlock = nSet;
                        }
                    }

    bool    IsSmall() const {return bIsSmall;}
    void        SetSmall(bool bSet){
                        if(bIsSmall != bSet)
                        {
                            SetModified();
                            bIsSmall = bSet;
                        }
                    }

    bool    IsGlobalActive() const {return bIsGlobalActive;}
    void        SetGlobalActive(bool bSet){
                        if(bIsGlobalActive != bSet)
                        {
                            SetModified();
                            bIsGlobalActive = bSet;
                        }
                    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
