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


#ifndef _LABEL_HXX
#define _LABEL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <labelcfg.hxx>
class SwLabRec;
class SwLabRecs;
class SwLabItem;
class SwLabPrtPage;
class SwNewDBMgr;
class Printer;

class SwLabDlg : public SfxTabDialog
{
    SwLabelConfig   aLabelsCfg;
    SwNewDBMgr*     pNewDBMgr;
    SwLabPrtPage*   pPrtPage;

    SvUShorts       aTypeIds;
    SvStringsDtor   aMakes;

    SwLabRecs*      pRecs;
    String          aLstGroup;
    String          sBusinessCardDlg;
    String          sFormat;
    String          sMedium;
    sal_Bool            m_bLabel;
    void          _ReplaceGroup( const String &rMake );

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
public:

     SwLabDlg( Window* pParent, const SfxItemSet& rSet,
                 SwNewDBMgr* pNewDBMgr, sal_Bool bLabel);
    ~SwLabDlg();

    SwLabRec*   GetRecord(const String &rRecName, sal_Bool bCont);
    void        GetLabItem(SwLabItem &rItem);

          SwLabRecs &Recs()           { return *pRecs;   }
    const SwLabRecs &Recs()     const { return *pRecs;   }

          SvUShorts  &TypeIds()       { return aTypeIds; }
    const SvUShorts  &TypeIds() const { return aTypeIds; }

          SvStringsDtor  &Makes()         { return aMakes;   }
    const SvStringsDtor  &Makes()   const { return aMakes;   }

    Printer *GetPrt();
    inline void ReplaceGroup( const String &rMake );
    void UpdateGroup( const String &rMake ) {_ReplaceGroup( rMake );}
    static void UpdateFieldInformation(::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
                                                                                const SwLabItem& rItem);
    const String& GetBusinessCardStr() const {return sBusinessCardDlg;}

    SwLabelConfig& GetLabelsConfig() {return aLabelsCfg;}

};

inline void SwLabDlg::ReplaceGroup( const String &rMake )
{
    if ( rMake != aLstGroup )
        _ReplaceGroup( rMake );
}

#endif

