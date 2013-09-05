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

#include "addinhelpid.hxx"
#include "sc.hrc"

// A struct containing the built-in function name and the built-in help ID.
struct ScUnoAddInHelpId
{
    const sal_Char*             pFuncName;
    const sal_Char*             sHelpId;
};

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
    { "getImcosh"                   , HID_AAI_FUNC_IMCOSH           },
    { "getImcot"                    , HID_AAI_FUNC_IMCOT            },
    { "getImcsc"                    , HID_AAI_FUNC_IMCSC            },
    { "getImcsch"                   , HID_AAI_FUNC_IMCSCH           },
    { "getImdiv"                    , HID_AAI_FUNC_IMDIV            },
    { "getImexp"                    , HID_AAI_FUNC_IMEXP            },
    { "getImln"                     , HID_AAI_FUNC_IMLN             },
    { "getImlog10"                  , HID_AAI_FUNC_IMLOG10          },
    { "getImlog2"                   , HID_AAI_FUNC_IMLOG2           },
    { "getImpower"                  , HID_AAI_FUNC_IMPOWER          },
    { "getImproduct"                , HID_AAI_FUNC_IMPRODUCT        },
    { "getImreal"                   , HID_AAI_FUNC_IMREAL           },
    { "getImsec"                    , HID_AAI_FUNC_IMSEC            },
    { "getImsech"                   , HID_AAI_FUNC_IMSECH           },
    { "getImsin"                    , HID_AAI_FUNC_IMSIN            },
    { "getImsinh"                   , HID_AAI_FUNC_IMSINH           },
    { "getImsqrt"                   , HID_AAI_FUNC_IMSQRT           },
    { "getImsub"                    , HID_AAI_FUNC_IMSUB            },
    { "getImsum"                    , HID_AAI_FUNC_IMSUM            },
    { "getImtan"                    , HID_AAI_FUNC_IMTAN            },
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

ScUnoAddInHelpIdGenerator::ScUnoAddInHelpIdGenerator( const OUString& rServiceName )
{
    SetServiceName( rServiceName );
}

void ScUnoAddInHelpIdGenerator::SetServiceName( const OUString& rServiceName )
{
    pCurrHelpIds = NULL;
    sal_uInt32 nSize = 0;

    if ( rServiceName == "com.sun.star.sheet.addin.Analysis" )
    {
        pCurrHelpIds = pAnalysisHelpIds;
        nSize = sizeof( pAnalysisHelpIds );
    }
    else if ( rServiceName == "com.sun.star.sheet.addin.DateFunctions" )
    {
        pCurrHelpIds = pDateFuncHelpIds;
        nSize = sizeof( pDateFuncHelpIds );
    }

    nArrayCount = nSize / sizeof( ScUnoAddInHelpId );
}

OString ScUnoAddInHelpIdGenerator::GetHelpId( const OUString& rFuncName ) const
{
    if( !pCurrHelpIds || !nArrayCount )
        return OString();

    const ScUnoAddInHelpId* pFirst = pCurrHelpIds;
    const ScUnoAddInHelpId* pLast = pCurrHelpIds + nArrayCount - 1;

    while( pFirst <= pLast )
    {
        const ScUnoAddInHelpId* pMiddle = pFirst + (pLast - pFirst) / 2;
        sal_Int32 nResult = rFuncName.compareToAscii( pMiddle->pFuncName );
        if( !nResult )
            return pMiddle->sHelpId;
        else if( nResult < 0 )
            pLast = pMiddle - 1;
        else
            pFirst = pMiddle + 1;
    }

    return OString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
