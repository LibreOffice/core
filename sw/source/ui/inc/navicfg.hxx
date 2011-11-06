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


#ifndef _NAVICFG_HXX
#define _NAVICFG_HXX

#include <unotools/configitem.hxx>
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

