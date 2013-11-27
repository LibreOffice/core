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


#ifndef _FMTFLD_HXX
#define _FMTFLD_HXX

#include <list>
#include <svl/poolitem.hxx>
#include <svl/brdcst.hxx>
#include <svl/smplhint.hxx>

#include "swdllapi.h"
#include <calbck.hxx>

class SwField;
class SwTxtFld;
class SwView;
class SwFieldType;

// ATT_FLD ***********************************
class SW_DLLPUBLIC SwFmtFld : public SfxPoolItem, public SwClient, public SfxBroadcaster
{
    friend void _InitCore();
    SwFmtFld( sal_uInt16 nWhich ); // for default-Attibute

    SwField* mpField;
    SwTxtFld* mpTxtFld; // the TextAttribute


    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld& operator=(const SwFmtFld& rFld);

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
    virtual void SwClientNotify( const SwModify& rModify, const SfxHint& rHint );

public:
    // single argument constructors shall be explicit.
    explicit SwFmtFld( const SwField &rFld );

    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld( const SwFmtFld& rAttr );

    virtual ~SwFmtFld();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual sal_Bool GetInfo( SfxPoolItem& rInfo ) const;

    const SwField* GetField() const
    {
        return mpField;
    }
    SwField* GetField()
    {
        return mpField;
    }

    // #111840#
    /**
       Sets current field.

       @param pField          new field

       @attention The current field will be destroyed before setting the new field.
     */
    void SetField( SwField * pField );


    const SwTxtFld* GetTxtFld() const
    {
        return mpTxtFld;
    }
    SwTxtFld* GetTxtFld()
    {
        return mpTxtFld;
    }
    void SetTxtFld( SwTxtFld& rTxtFld );
    void ClearTxtFld();


    void RegisterToFieldType( SwFieldType& );
    bool IsFldInDoc() const;
    sal_Bool IsProtect() const;
};

class SW_DLLPUBLIC SwFmtFldHint : public SfxHint
{
#define SWFMTFLD_INSERTED   1
#define SWFMTFLD_REMOVED    2
#define SWFMTFLD_FOCUS      3
#define SWFMTFLD_CHANGED    4
#define SWFMTFLD_LANGUAGE   5

    const SwFmtFld* pFld;
    sal_Int16 nWhich;
    const SwView* pView;

public:
    SwFmtFldHint( const SwFmtFld* p, sal_Int16 n, const SwView* pV = 0)
        : pFld(p)
        , nWhich(n)
        , pView(pV)
    {}

    const SwFmtFld* GetField() const { return pFld; }
    sal_Int16 Which() const { return nWhich; }
    const SwView* GetView() const { return pView; }
};

#endif
