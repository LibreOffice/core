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
#include <o3tl/enumarray.hxx>
#include "swcont.hxx"

class SwNavigationConfig final : public utl::ConfigItem
{
    ContentTypeId  m_nRootType;      //RootType
    sal_Int32      m_nSelectedPos;   //SelectedPosition
    sal_Int32      m_nOutlineLevel;  //OutlineLevel
    sal_Int32      m_nActiveBlock;   //ActiveBlock//Expand/CollapsState
    bool           m_bIsSmall;       //ShowListBox
    bool           m_bIsGlobalActive; //GlobalDocMode// global view for GlobalDoc valid?
    sal_Int32      m_nOutlineTracking;
    bool           m_bIsNavigateOnSelect;

    o3tl::enumarray<ContentTypeId, bool> mContentTypeTrack;

    sal_Int32 m_nSortAlphabeticallyBlock = 0; // persists content type alphabetical sort setting

    static css::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwNavigationConfig();
    virtual ~SwNavigationConfig() override;

    void Load();
    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    ContentTypeId GetRootType()const {return m_nRootType;}
    void        SetRootType(ContentTypeId nSet){
                        if(m_nRootType != nSet)
                        {
                            SetModified();
                            m_nRootType = nSet;
                        }
                    }

    sal_Int32   GetOutlineLevel()const {return m_nOutlineLevel;}
    void        SetOutlineLevel(sal_Int32 nSet){
                        if(m_nOutlineLevel != nSet)
                        {
                            SetModified();
                            m_nOutlineLevel = nSet;
                        }
                    }

    sal_Int32   GetActiveBlock()const {return m_nActiveBlock;}
    void        SetActiveBlock(sal_Int32 nSet){
                        if(m_nActiveBlock != nSet)
                        {
                            SetModified();
                            m_nActiveBlock = nSet;
                        }
                    }

    bool    IsSmall() const {return m_bIsSmall;}
    void        SetSmall(bool bSet){
                        if(m_bIsSmall != bSet)
                        {
                            SetModified();
                            m_bIsSmall = bSet;
                        }
                    }

    bool    IsGlobalActive() const {return m_bIsGlobalActive;}
    void        SetGlobalActive(bool bSet){
                        if(m_bIsGlobalActive != bSet)
                        {
                            SetModified();
                            m_bIsGlobalActive = bSet;
                        }
                    }

    sal_Int32   GetOutlineTracking()const {return m_nOutlineTracking;}
    void        SetOutlineTracking(sal_Int32 nSet){
                        if(m_nOutlineTracking != nSet)
                        {
                            SetModified();
                            m_nOutlineTracking = nSet;
                        }
                    }

    bool IsContentTypeTrack(ContentTypeId eCntTypeId)
    {
        return mContentTypeTrack[eCntTypeId];
    }
    void SetContentTypeTrack(ContentTypeId eCntTypeId, const bool bSet)
    {
        if (mContentTypeTrack[eCntTypeId] != bSet)
        {
            SetModified();
            mContentTypeTrack[eCntTypeId] = bSet;
        }
    }

    bool    IsNavigateOnSelect() const {return m_bIsNavigateOnSelect;}

    sal_Int32 GetSortAlphabeticallyBlock() const {return m_nSortAlphabeticallyBlock;}
    void SetSortAlphabeticallyBlock(sal_Int32 nSet)
    {
        if(m_nSortAlphabeticallyBlock != nSet)
        {
            SetModified();
            m_nSortAlphabeticallyBlock = nSet;
        }
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
