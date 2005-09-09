/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navicfg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:51:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _NAVICFG_HXX
#define _NAVICFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
/* -----------------------------08.09.00 16:27--------------------------------

 ---------------------------------------------------------------------------*/
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

    virtual void            Commit();

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

