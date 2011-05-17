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
#ifndef _NAVICFG_HXX
#define _NAVICFG_HXX

#include <unotools/configitem.hxx>

class SwNavigationConfig : public utl::ConfigItem
{
    sal_Int32   nRootType;      //RootType
    sal_Int32   nSelectedPos;   //SelectedPosition
    sal_Int32   nOutlineLevel;  //OutlineLevel
    sal_Int32   nRegionMode;    //InsertMode
    sal_Int32   nActiveBlock;   //ActiveBlock//Expand/CollapsState
    sal_Bool    bIsSmall;       //ShowListBox
    sal_Bool    bIsGlobalActive; //GlobalDocMode// Globalansicht fuer GlobalDoc gueltig?

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
    SwNavigationConfig();
    ~SwNavigationConfig();

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    sal_Int32   GetRootType()const {return nRootType;}
    void        SetRootType(sal_Int32 nSet){
                        if(nRootType != nSet)
                        {
                            SetModified();
                            nRootType = nSet;
                        }
                    }

    sal_Int32   GetSelectedPos()const {return nSelectedPos;}
    void        SetSelectedPos(sal_Int32 nSet){
                        if(nSelectedPos != nSet)
                        {
                            SetModified();
                            nSelectedPos = nSet;
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

    sal_Int32   GetRegionMode()const {return nRegionMode;}
    void        SetRegionMode(sal_Int32 nSet){
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

    sal_Bool    IsSmall() const {return bIsSmall;}
    void        SetSmall(sal_Bool bSet){
                        if(bIsSmall != bSet)
                        {
                            SetModified();
                            bIsSmall = bSet;
                        }
                    }

    sal_Bool    IsGlobalActive() const {return bIsGlobalActive;}
    void        SetGlobalActive(sal_Bool bSet){
                        if(bIsGlobalActive != bSet)
                        {
                            SetModified();
                            bIsGlobalActive = bSet;
                        }
                    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
