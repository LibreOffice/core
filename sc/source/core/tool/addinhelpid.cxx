/*************************************************************************
 *
 *  $RCSfile: addinhelpid.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2002-05-02 10:37:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// ============================================================================

#ifndef SC_ADDINHELPID_HXX
#include "addinhelpid.hxx"
#endif

#ifndef SC_SC_HRC
#include "sc.hrc"
#endif


// ============================================================================

// A struct containing the built-in function name and the built-in help ID.
struct ScUnoAddInHelpId
{
    const sal_Char*             pFuncName;
    sal_uInt16                  nHelpId;
};


// ----------------------------------------------------------------------------

// Help IDs for Analysis AddIn. MUST BE SORTED for binary search.
const ScUnoAddInHelpId pAnalysisHelpIds[] =
{
    { "getAccrint"                  , HID_AAI_FUNC_ACCRINT          },
    { "getAccrintm"                 , HID_AAI_FUNC_ACCRINTM         },
    { "getAmordegrc"                , HID_AAI_FUNC_AMORDEGRC        },
    { "getAmorlinc"                 , HID_AAI_FUNC_AMORLINC         },
    { "getBesseli"                  , HID_AAI_FUNC_BESSELI          },
    { "getBesselj"                  , HID_AAI_FUNC_BESSELJ          },
    { "getBesselk"                  , HID_AAI_FUNC_BESSELK          },
    { "getBessely"                  , HID_AAI_FUNC_BESSELY          },
    { "getBin2Dec"                  , HID_AAI_FUNC_BIN2DEC          },
    { "getBin2Hex"                  , HID_AAI_FUNC_BIN2HEX          },
    { "getBin2Oct"                  , HID_AAI_FUNC_BIN2OCT          },
    { "getComplex"                  , HID_AAI_FUNC_COMPLEX          },
    { "getConvert"                  , HID_AAI_FUNC_CONVERT          },
    { "getCoupdaybs"                , HID_AAI_FUNC_COUPDAYBS        },
    { "getCoupdays"                 , HID_AAI_FUNC_COUPDAYS         },
    { "getCoupdaysnc"               , HID_AAI_FUNC_COUPDAYSNC       },
    { "getCoupncd"                  , HID_AAI_FUNC_COUPNCD          },
    { "getCoupnum"                  , HID_AAI_FUNC_COUPNUM          },
    { "getCouppcd"                  , HID_AAI_FUNC_COUPPCD          },
    { "getCumipmt"                  , HID_AAI_FUNC_CUMIPMT          },
    { "getCumprinc"                 , HID_AAI_FUNC_CUMPRINC         },
    { "getDec2Bin"                  , HID_AAI_FUNC_DEC2BIN          },
    { "getDec2Hex"                  , HID_AAI_FUNC_DEC2HEX          },
    { "getDec2Oct"                  , HID_AAI_FUNC_DEC2OCT          },
    { "getDelta"                    , HID_AAI_FUNC_DELTA            },
    { "getDisc"                     , HID_AAI_FUNC_DISC             },
    { "getDollarde"                 , HID_AAI_FUNC_DOLLARDE         },
    { "getDollarfr"                 , HID_AAI_FUNC_DOLLARFR         },
    { "getDuration"                 , HID_AAI_FUNC_DURATION         },
    { "getEdate"                    , HID_AAI_FUNC_EDATE            },
    { "getEffect"                   , HID_AAI_FUNC_EFFECT           },
    { "getEomonth"                  , HID_AAI_FUNC_EOMONTH          },
    { "getErf"                      , HID_AAI_FUNC_ERF              },
    { "getErfc"                     , HID_AAI_FUNC_ERFC             },
    { "getFactdouble"               , HID_AAI_FUNC_FACTDOUBLE       },
    { "getFvschedule"               , HID_AAI_FUNC_FVSCHEDULE       },
    { "getGcd"                      , HID_AAI_FUNC_GCD              },
    { "getGestep"                   , HID_AAI_FUNC_GESTEP           },
    { "getHex2Bin"                  , HID_AAI_FUNC_HEX2BIN          },
    { "getHex2Dec"                  , HID_AAI_FUNC_HEX2DEC          },
    { "getHex2Oct"                  , HID_AAI_FUNC_HEX2OCT          },
    { "getImabs"                    , HID_AAI_FUNC_IMABS            },
    { "getImaginary"                , HID_AAI_FUNC_IMAGINARY        },
    { "getImargument"               , HID_AAI_FUNC_IMARGUMENT       },
    { "getImconjugate"              , HID_AAI_FUNC_IMCONJUGATE      },
    { "getImcos"                    , HID_AAI_FUNC_IMCOS            },
    { "getImdiv"                    , HID_AAI_FUNC_IMDIV            },
    { "getImexp"                    , HID_AAI_FUNC_IMEXP            },
    { "getImln"                     , HID_AAI_FUNC_IMLN             },
    { "getImlog10"                  , HID_AAI_FUNC_IMLOG10          },
    { "getImlog2"                   , HID_AAI_FUNC_IMLOG2           },
    { "getImpower"                  , HID_AAI_FUNC_IMPOWER          },
    { "getImproduct"                , HID_AAI_FUNC_IMPRODUCT        },
    { "getImreal"                   , HID_AAI_FUNC_IMREAL           },
    { "getImsin"                    , HID_AAI_FUNC_IMSIN            },
    { "getImsqrt"                   , HID_AAI_FUNC_IMSQRT           },
    { "getImsub"                    , HID_AAI_FUNC_IMSUB            },
    { "getImsum"                    , HID_AAI_FUNC_IMSUM            },
    { "getIntrate"                  , HID_AAI_FUNC_INTRATE          },
    { "getIseven"                   , HID_AAI_FUNC_ISEVEN           },
    { "getIsodd"                    , HID_AAI_FUNC_ISODD            },
    { "getLcm"                      , HID_AAI_FUNC_LCM              },
    { "getMduration"                , HID_AAI_FUNC_MDURATION        },
    { "getMround"                   , HID_AAI_FUNC_MROUND           },
    { "getMultinomial"              , HID_AAI_FUNC_MULTINOMIAL      },
    { "getNetworkdays"              , HID_AAI_FUNC_NETWORKDAYS      },
    { "getNominal"                  , HID_AAI_FUNC_NOMINAL          },
    { "getOct2Bin"                  , HID_AAI_FUNC_OCT2BIN          },
    { "getOct2Dec"                  , HID_AAI_FUNC_OCT2DEZ          },
    { "getOct2Hex"                  , HID_AAI_FUNC_OCT2HEX          },
    { "getOddfprice"                , HID_AAI_FUNC_ODDFPRICE        },
    { "getOddfyield"                , HID_AAI_FUNC_ODDFYIELD        },
    { "getOddlprice"                , HID_AAI_FUNC_ODDLPRICE        },
    { "getOddlyield"                , HID_AAI_FUNC_ODDLYIELD        },
    { "getPrice"                    , HID_AAI_FUNC_PRICE            },
    { "getPricedisc"                , HID_AAI_FUNC_PRICEDISC        },
    { "getPricemat"                 , HID_AAI_FUNC_PRICEMAT         },
    { "getQuotient"                 , HID_AAI_FUNC_QUOTIENT         },
    { "getRandbetween"              , HID_AAI_FUNC_RANDBETWEEN      },
    { "getReceived"                 , HID_AAI_FUNC_RECEIVED         },
    { "getSeriessum"                , HID_AAI_FUNC_SERIESSUM        },
    { "getSqrtpi"                   , HID_AAI_FUNC_SQRTPI           },
    { "getTbilleq"                  , HID_AAI_FUNC_TBILLEQ          },
    { "getTbillprice"               , HID_AAI_FUNC_TBILLPRICE       },
    { "getTbillyield"               , HID_AAI_FUNC_TBILLYIELD       },
    { "getWeeknum"                  , HID_AAI_FUNC_WEEKNUM          },
    { "getWorkday"                  , HID_AAI_FUNC_WORKDAY          },
    { "getXirr"                     , HID_AAI_FUNC_XIRR             },
    { "getXnpv"                     , HID_AAI_FUNC_XNPV             },
    { "getYearfrac"                 , HID_AAI_FUNC_YEARFRAC         },
    { "getYield"                    , HID_AAI_FUNC_YIELD            },
    { "getYielddisc"                , HID_AAI_FUNC_YIELDDISC        },
    { "getYieldmat"                 , HID_AAI_FUNC_YIELDMAT         }
};


// ----------------------------------------------------------------------------

// Help IDs for DateFunc AddIn. MUST BE SORTED for binary search.
const ScUnoAddInHelpId pDateFuncHelpIds[] =
{
    { "getDaysInMonth"              , HID_DAI_FUNC_DAYSINMONTH      },
    { "getDaysInYear"               , HID_DAI_FUNC_DAYSINYEAR       },
    { "getDiffMonths"               , HID_DAI_FUNC_DIFFMONTHS       },
    { "getDiffWeeks"                , HID_DAI_FUNC_DIFFWEEKS        },
    { "getDiffYears"                , HID_DAI_FUNC_DIFFYEARS        },
    { "getRot13"                    , HID_DAI_FUNC_ROT13            },
    { "getWeeksInYear"              , HID_DAI_FUNC_WEEKSINYEAR      }
};


// ============================================================================

ScUnoAddInHelpIdGenerator::ScUnoAddInHelpIdGenerator() :
    pCurrHelpIds( NULL ),
    nArrayCount( 0 )
{
}

ScUnoAddInHelpIdGenerator::ScUnoAddInHelpIdGenerator( const ::rtl::OUString& rServiceName )
{
    SetServiceName( rServiceName );
}

void ScUnoAddInHelpIdGenerator::SetServiceName( const ::rtl::OUString& rServiceName )
{
    pCurrHelpIds = NULL;
    sal_uInt32 nSize = 0;

    if( rServiceName.equalsAscii( "com.sun.star.sheet.addin.Analysis" ) )
    {
        pCurrHelpIds = pAnalysisHelpIds;
        nSize = sizeof( pAnalysisHelpIds );
    }
    else if( rServiceName.equalsAscii( "com.sun.star.sheet.addin.DateFunctions" ) )
    {
        pCurrHelpIds = pDateFuncHelpIds;
        nSize = sizeof( pDateFuncHelpIds );
    }

    nArrayCount = nSize / sizeof( ScUnoAddInHelpId );
}

sal_uInt16 ScUnoAddInHelpIdGenerator::GetHelpId( const ::rtl::OUString& rFuncName ) const
{
    if( !pCurrHelpIds || !nArrayCount )
        return 0;

    const ScUnoAddInHelpId* pFirst = pCurrHelpIds;
    const ScUnoAddInHelpId* pLast = pCurrHelpIds + nArrayCount - 1;

    while( pFirst <= pLast )
    {
        const ScUnoAddInHelpId* pMiddle = pFirst + (pLast - pFirst) / 2;
        sal_Int32 nResult = rFuncName.compareToAscii( pMiddle->pFuncName );
        if( !nResult )
            return pMiddle->nHelpId;
        else if( nResult < 0 )
            pLast = pMiddle - 1;
        else
            pFirst = pMiddle + 1;
    }

    return 0;
}


// ============================================================================

