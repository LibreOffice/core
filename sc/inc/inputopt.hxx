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



#ifndef SC_INPUTOPT_HXX
#define SC_INPUTOPT_HXX


#include <unotools/configitem.hxx>
#include <tools/solar.h>

class SvStream;


class ScInputOptions
{
private:
    sal_uInt16      nMoveDir;           // enum ScDirection
    sal_Bool        bMoveSelection;
    sal_Bool        bEnterEdit;
    sal_Bool        bExtendFormat;
    sal_Bool        bRangeFinder;
    sal_Bool        bExpandRefs;
    sal_Bool        bMarkHeader;
    sal_Bool        bUseTabCol;
    sal_Bool        bTextWysiwyg;
    sal_Bool        bReplCellsWarn;

public:
                ScInputOptions();
                ScInputOptions( const ScInputOptions& rCpy );
                ~ScInputOptions();

    void        SetDefaults();

    void        SetMoveDir(sal_uInt16 nNew)         { nMoveDir = nNew;       }
    sal_uInt16      GetMoveDir() const              { return nMoveDir;       }
    void        SetMoveSelection(sal_Bool bSet)     { bMoveSelection = bSet; }
    sal_Bool        GetMoveSelection() const        { return bMoveSelection; }
    void        SetEnterEdit(sal_Bool bSet)         { bEnterEdit = bSet;     }
    sal_Bool        GetEnterEdit() const            { return bEnterEdit;     }
    void        SetExtendFormat(sal_Bool bSet)      { bExtendFormat = bSet;  }
    sal_Bool        GetExtendFormat() const         { return bExtendFormat;  }
    void        SetRangeFinder(sal_Bool bSet)       { bRangeFinder = bSet;   }
    sal_Bool        GetRangeFinder() const          { return bRangeFinder;   }
    void        SetExpandRefs(sal_Bool bSet)        { bExpandRefs = bSet;    }
    sal_Bool        GetExpandRefs() const           { return bExpandRefs;    }
    void        SetMarkHeader(sal_Bool bSet)        { bMarkHeader = bSet;    }
    sal_Bool        GetMarkHeader() const           { return bMarkHeader;    }
    void        SetUseTabCol(sal_Bool bSet)         { bUseTabCol = bSet;     }
    sal_Bool        GetUseTabCol() const            { return bUseTabCol;     }
    void        SetTextWysiwyg(sal_Bool bSet)       { bTextWysiwyg = bSet;   }
    sal_Bool        GetTextWysiwyg() const          { return bTextWysiwyg;   }
    void        SetReplaceCellsWarn(sal_Bool bSet)  { bReplCellsWarn = bSet; }
    sal_Bool        GetReplaceCellsWarn() const     { return bReplCellsWarn; }

    const ScInputOptions&   operator=   ( const ScInputOptions& rOpt );
};


//==================================================================
// CfgItem fuer Eingabe-Optionen
//==================================================================

class ScInputCfg : public ScInputOptions,
                  public utl::ConfigItem
{
    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
            ScInputCfg();

    void            SetOptions( const ScInputOptions& rNew );
    void            OptionsChanged();   // after direct access to SetOptions base class

    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
    virtual void    Commit();
};


#endif

