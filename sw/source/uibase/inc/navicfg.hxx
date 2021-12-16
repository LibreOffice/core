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

class SwNavigationConfig final : public utl::ConfigItem
{
    ContentTypeId  m_nRootType;      //RootType
    sal_Int32      m_nSelectedPos;   //SelectedPosition
    sal_Int32      m_nOutlineLevel;  //OutlineLevel
    RegionMode     m_nRegionMode;    //InsertMode
    sal_Int32      m_nActiveBlock;   //ActiveBlock//Expand/CollapsState
    bool           m_bIsSmall;       //ShowListBox
    bool           m_bIsGlobalActive; //GlobalDocMode// global view for GlobalDoc valid?
    sal_Int32      m_nOutlineTracking;
    bool           m_bIsTableTracking;
    bool           m_bIsSectionTracking;
    bool           m_bIsFrameTracking;
    bool           m_bIsImageTracking;
    bool           m_bIsOLEobjectTracking;
    bool           m_bIsBookmarkTracking;
    bool           m_bIsHyperlinkTracking;
    bool           m_bIsReferenceTracking;
    bool           m_bIsIndexTracking;
    bool           m_bIsCommentTracking;
    bool           m_bIsDrawingObjectTracking;
    bool           m_bIsFieldTracking;
    bool           m_bIsFootnoteTracking;
    bool           m_bIsNavigateOnSelect;

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

    RegionMode  GetRegionMode()const {return m_nRegionMode;}
    void        SetRegionMode(RegionMode nSet){
                    if(m_nRegionMode != nSet)
                    {
                        SetModified();
                        m_nRegionMode = nSet;
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

    bool    IsTableTracking() const {return m_bIsTableTracking;}
    void    SetTableTracking(bool bSet){
                        if(m_bIsTableTracking != bSet)
                        {
                            SetModified();
                            m_bIsTableTracking = bSet;
                        }
                    }

    bool    IsSectionTracking() const {return m_bIsSectionTracking;}
    void    SetSectionTracking(bool bSet){
                        if(m_bIsSectionTracking != bSet)
                        {
                            SetModified();
                            m_bIsSectionTracking = bSet;
                        }
                    }

    bool    IsFrameTracking() const {return m_bIsFrameTracking;}
    void    SetFrameTracking(bool bSet){
                        if(m_bIsFrameTracking != bSet)
                        {
                            SetModified();
                            m_bIsFrameTracking = bSet;
                        }
                    }

    bool    IsImageTracking() const {return m_bIsImageTracking;}
    void    SetImageTracking(bool bSet){
                        if(m_bIsImageTracking != bSet)
                        {
                            SetModified();
                            m_bIsImageTracking = bSet;
                        }
                    }

    bool    IsOLEobjectTracking() const {return m_bIsOLEobjectTracking;}
    void    SetOLEobjectTracking(bool bSet){
                        if(m_bIsOLEobjectTracking != bSet)
                        {
                            SetModified();
                            m_bIsOLEobjectTracking = bSet;
                        }
                    }

    bool    IsBookmarkTracking() const {return m_bIsBookmarkTracking;}
    void    SetBookmarkTracking(bool bSet){
                        if(m_bIsBookmarkTracking != bSet)
                        {
                            SetModified();
                            m_bIsBookmarkTracking = bSet;
                        }
                    }

    bool    IsHyperlinkTracking() const {return m_bIsHyperlinkTracking;}
    void    SetHyperlinkTracking(bool bSet){
                        if(m_bIsHyperlinkTracking != bSet)
                        {
                            SetModified();
                            m_bIsHyperlinkTracking = bSet;
                        }
                    }

    bool    IsReferenceTracking() const {return m_bIsReferenceTracking;}
    void    SetReferenceTracking(bool bSet){
                        if(m_bIsReferenceTracking != bSet)
                        {
                            SetModified();
                            m_bIsReferenceTracking = bSet;
                        }
                    }

    bool    IsIndexTracking() const {return m_bIsIndexTracking;}
    void    SetIndexTracking(bool bSet){
                        if(m_bIsIndexTracking != bSet)
                        {
                            SetModified();
                            m_bIsIndexTracking = bSet;
                        }
                    }

    bool    IsCommentTracking() const {return m_bIsCommentTracking;}
    void    SetCommentTracking(bool bSet){
                        if(m_bIsCommentTracking != bSet)
                        {
                            SetModified();
                            m_bIsCommentTracking = bSet;
                        }
                    }

    bool    IsDrawingObjectTracking() const {return m_bIsDrawingObjectTracking;}
    void    SetDrawingObjectTracking(bool bSet){
                        if(m_bIsDrawingObjectTracking != bSet)
                        {
                            SetModified();
                            m_bIsDrawingObjectTracking = bSet;
                        }
                    }

    bool    IsFieldTracking() const {return m_bIsFieldTracking;}
    void    SetFieldTracking(bool bSet){
                        if(m_bIsFieldTracking != bSet)
                        {
                            SetModified();
                            m_bIsFieldTracking = bSet;
                        }
                    }

    bool    IsFootnoteTracking() const {return m_bIsFootnoteTracking;}
    void    SetFootnoteTracking(bool bSet){
                        if(m_bIsFootnoteTracking != bSet)
                        {
                            SetModified();
                            m_bIsFootnoteTracking = bSet;
                        }
                    }

    bool    IsNavigateOnSelect() const {return m_bIsNavigateOnSelect;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
