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

#include <unofield.hxx>
#include <unofieldcoll.hxx>
#include <swtypes.hxx>
#include <cmdid.h>
#include <doc.hxx>
#include <hints.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unocoll.hxx>
#include <sfx2/linkmgr.hxx>
#include <docstat.hxx>
#include <editsh.hxx>
#include <viewsh.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>

//undef to prevent error (from sfx2/docfile.cxx)
#undef SEQUENCE
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <unocrsr.hxx>
#include <authfld.hxx>
#include <flddat.hxx>
#include <dbfld.hxx>
#include <usrfld.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <chpfld.hxx>
#include <flddropdown.hxx>
#include <poolfmt.hxx>
#include <poolfmt.hrc>
#include <pagedesc.hxx>
#include <docary.hxx>
#include <reffld.hxx>
#include <ddefld.hxx>
#include <SwStyleNameMapper.hxx>
#include <swunohelper.hxx>
#include <unofldmid.h>
#include <scriptinfo.hxx>
#include <tools/datetime.hxx>
#include <tools/urlobj.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <textapi.hxx>
#include <editeng/outliner.hxx>
#include <docsh.hxx>
#include <fmtmeta.hxx> // MetaFieldManager
#include <switerator.hxx>
#include <bookmrk.hxx>
#include <rtl/strbuf.hxx>
#include <vector>
#include <xmloff/odffields.hxx>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

#define COM_TEXT_FLDMASTER      "com.sun.star.text.FieldMaster."

// case-corrected version of the first part for the service names (see #i67811)
#define COM_TEXT_FLDMASTER_CC   "com.sun.star.text.fieldmaster."

// note: this thing is indexed as an array, so do not insert/remove entries!
static const sal_uInt16 aDocInfoSubTypeFromService[] =
{
    DI_CHANGE | DI_SUB_AUTHOR,  //PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_AUTHOR
    DI_CHANGE | DI_SUB_DATE,    //PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME
    DI_EDIT | DI_SUB_TIME,      //PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME
    DI_COMMENT,                 //PROPERTY_MAP_FLDTYP_DOCINFO_DESCRIPTION
    DI_CREATE | DI_SUB_AUTHOR,  //PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_AUTHOR
    DI_CREATE | DI_SUB_DATE,    //PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME
    0,                          //DUMMY
    0,                          //DUMMY
    0,                          //DUMMY
    0,                          //DUMMY
    DI_CUSTOM,                  //PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM
    DI_PRINT | DI_SUB_AUTHOR,   //PROPERTY_MAP_FLDTYP_DOCINFO_PRINT_AUTHOR
    DI_PRINT | DI_SUB_DATE,     //PROPERTY_MAP_FLDTYP_DOCINFO_PRINT_DATE_TIME
    DI_KEYS,                    //PROPERTY_MAP_FLDTYP_DOCINFO_KEY_WORDS
    DI_THEMA,                   //PROPERTY_MAP_FLDTYP_DOCINFO_SUBJECT
    DI_TITEL,                   //PROPERTY_MAP_FLDTYP_DOCINFO_TITLE
    DI_DOCNO                    //PROPERTY_MAP_FLDTYP_DOCINFO_REVISION
};

struct ServiceIdResId
{
    sal_uInt16 nResId;
    sal_uInt16 nServiceId;
};

static const ServiceIdResId aServiceToRes[] =
{
    {RES_DATETIMEFLD,   SW_SERVICE_FIELDTYPE_DATETIME               },
    {RES_USERFLD,       SW_SERVICE_FIELDTYPE_USER                   },
    {RES_SETEXPFLD,         SW_SERVICE_FIELDTYPE_SET_EXP            }    ,
    {RES_GETEXPFLD,         SW_SERVICE_FIELDTYPE_GET_EXP            }    ,
    {RES_FILENAMEFLD,   SW_SERVICE_FIELDTYPE_FILE_NAME              },
    {RES_PAGENUMBERFLD,     SW_SERVICE_FIELDTYPE_PAGE_NUM           }    ,
    {RES_AUTHORFLD,         SW_SERVICE_FIELDTYPE_AUTHOR             }    ,
    {RES_CHAPTERFLD,    SW_SERVICE_FIELDTYPE_CHAPTER                },
    {RES_GETREFFLD,         SW_SERVICE_FIELDTYPE_GET_REFERENCE      }    ,
    {RES_HIDDENTXTFLD,  SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT       },
    {RES_POSTITFLD,         SW_SERVICE_FIELDTYPE_ANNOTATION         }    ,
    {RES_INPUTFLD,      SW_SERVICE_FIELDTYPE_INPUT                  },
    {RES_MACROFLD,      SW_SERVICE_FIELDTYPE_MACRO                  },
    {RES_DDEFLD,        SW_SERVICE_FIELDTYPE_DDE                    },
    {RES_HIDDENPARAFLD,     SW_SERVICE_FIELDTYPE_HIDDEN_PARA        }    ,
    {RES_DOCINFOFLD,    SW_SERVICE_FIELDTYPE_DOC_INFO               },
    {RES_TEMPLNAMEFLD,  SW_SERVICE_FIELDTYPE_TEMPLATE_NAME          },
    {RES_EXTUSERFLD,    SW_SERVICE_FIELDTYPE_USER_EXT               },
    {RES_REFPAGESETFLD,     SW_SERVICE_FIELDTYPE_REF_PAGE_SET       }    ,
    {RES_REFPAGEGETFLD,     SW_SERVICE_FIELDTYPE_REF_PAGE_GET       }    ,
    {RES_JUMPEDITFLD,   SW_SERVICE_FIELDTYPE_JUMP_EDIT              },
    {RES_SCRIPTFLD,         SW_SERVICE_FIELDTYPE_SCRIPT             }    ,
    {RES_DBNEXTSETFLD,  SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET      },
    {RES_DBNUMSETFLD,   SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET       },
    {RES_DBSETNUMBERFLD, SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM      } ,
    {RES_DBFLD,             SW_SERVICE_FIELDTYPE_DATABASE           }    ,
    {RES_DBNAMEFLD,     SW_SERVICE_FIELDTYPE_DATABASE_NAME          },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_PAGE_COUNT             },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT        },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_WORD_COUNT             },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_CHARACTER_COUNT        },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_TABLE_COUNT            },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT   },
    {RES_DOCSTATFLD,    SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT  },
    {RES_DOCINFOFLD,    SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR  },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME},
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME       },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION     },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR   },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME},
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM          },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR    },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS       },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT         },
    {RES_DOCINFOFLD,        SW_SERVICE_FIELDTYPE_DOCINFO_TITLE           },
    {RES_INPUTFLD,      SW_SERVICE_FIELDTYPE_INPUT_USER                  },
    {RES_HIDDENTXTFLD,  SW_SERVICE_FIELDTYPE_HIDDEN_TEXT                 },
    {RES_AUTHORITY,     SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY                },
    {RES_COMBINED_CHARS,    SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS     },
    {RES_DROPDOWN,  SW_SERVICE_FIELDTYPE_DROPDOWN                        },
    {RES_TABLEFLD,      SW_SERVICE_FIELDTYPE_TABLE_FORMULA              },
    {USHRT_MAX,         USHRT_MAX                                        }
};

static sal_uInt16 lcl_ServiceIdToResId(sal_uInt16 nServiceId)
{
    const ServiceIdResId* pMap = aServiceToRes;
    while( USHRT_MAX != pMap->nServiceId && nServiceId != pMap->nServiceId )
            ++pMap;
#if OSL_DEBUG_LEVEL > 0
    if( USHRT_MAX == pMap->nServiceId )
        OSL_FAIL("service id not found");
#endif
    return pMap->nResId;
}

static sal_uInt16 lcl_GetServiceForField( const SwField& rFld )
{
    sal_uInt16 nWhich = rFld.Which(), nSrvId = USHRT_MAX;
    //special handling for some fields
    switch( nWhich )
    {
    case RES_INPUTFLD:
        if( INP_USR == (rFld.GetSubType() & 0x00ff) )
            nSrvId = SW_SERVICE_FIELDTYPE_INPUT_USER;
        break;

    case RES_DOCINFOFLD:
        {
            sal_uInt16 nSubType = rFld.GetSubType();
            switch( (nSubType & 0xff))
            {
            case DI_CHANGE:
                nSrvId = ((nSubType&0x300) == DI_SUB_AUTHOR)
                        ? SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR
                        : SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME;
                break;
            case DI_CREATE:
                nSrvId = ((nSubType&0x300) == DI_SUB_AUTHOR)
                        ? SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR
                        : SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME;
                break;
            case DI_PRINT:
                nSrvId = ((nSubType&0x300) == DI_SUB_AUTHOR)
                        ? SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR
                        : SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME;
                break;
            case DI_EDIT:   nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME;break;
            case DI_COMMENT:nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION;break;
            case DI_KEYS:   nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS;break;
            case DI_THEMA:  nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT;  break;
            case DI_TITEL:  nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_TITLE;    break;
            case DI_DOCNO:  nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_REVISION; break;
            case DI_CUSTOM: nSrvId = SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM;   break;
            }
        }
        break;

    case RES_HIDDENTXTFLD:
        nSrvId = TYP_CONDTXTFLD == rFld.GetSubType()
                        ? SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT
                        : SW_SERVICE_FIELDTYPE_HIDDEN_TEXT;
        break;

    case RES_DOCSTATFLD:
        {
            switch( rFld.GetSubType() )
            {
            case DS_PAGE: nSrvId = SW_SERVICE_FIELDTYPE_PAGE_COUNT; break;
            case DS_PARA: nSrvId = SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT; break;
            case DS_WORD: nSrvId = SW_SERVICE_FIELDTYPE_WORD_COUNT     ; break;
            case DS_CHAR: nSrvId = SW_SERVICE_FIELDTYPE_CHARACTER_COUNT; break;
            case DS_TBL:  nSrvId = SW_SERVICE_FIELDTYPE_TABLE_COUNT    ; break;
            case DS_GRF:  nSrvId = SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT; break;
            case DS_OLE:  nSrvId = SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT; break;
            }
        }
        break;
    }
    if( USHRT_MAX == nSrvId )
    {
        for( const ServiceIdResId* pMap = aServiceToRes;
                USHRT_MAX != pMap->nResId; ++pMap )
            if( nWhich == pMap->nResId )
            {
                nSrvId = pMap->nServiceId;
                break;
            }
    }
#if OSL_DEBUG_LEVEL > 0
    if( USHRT_MAX == nSrvId )
        OSL_FAIL("resid not found");
#endif
    return nSrvId;
}

static sal_uInt16 lcl_GetPropMapIdForFieldType( sal_uInt16 nWhich )
{
    sal_uInt16 nId;
    switch( nWhich )
    {
    case RES_USERFLD:   nId = PROPERTY_MAP_FLDMSTR_USER;            break;
    case RES_DBFLD:     nId = PROPERTY_MAP_FLDMSTR_DATABASE;        break;
    case RES_SETEXPFLD: nId = PROPERTY_MAP_FLDMSTR_SET_EXP;         break;
    case RES_DDEFLD:    nId = PROPERTY_MAP_FLDMSTR_DDE;             break;
    case RES_AUTHORITY: nId = PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY;    break;
    default:            nId = PROPERTY_MAP_FLDMSTR_DUMMY0;
    }
    return nId;
}

sal_uInt16 GetFieldTypeMId( const OUString& rProperty, const SwFieldType& rTyp )
{
    sal_uInt16 nId = lcl_GetPropMapIdForFieldType( rTyp.Which() );
    const SfxItemPropertySet* pSet = aSwMapProvider.GetPropertySet( nId );
    if( !pSet )
        nId = USHRT_MAX;
    else
    {
        const SfxItemPropertySimpleEntry* pEntry = pSet->getPropertyMap().getByName(rProperty);
        nId = pEntry ? pEntry->nWID : USHRT_MAX;
    }
    return nId;
}

static sal_uInt16 lcl_GetPropertyMapOfService( sal_uInt16 nServiceId )
{
    sal_uInt16 nRet;
    switch ( nServiceId)
    {
    case SW_SERVICE_FIELDTYPE_DATETIME: nRet = PROPERTY_MAP_FLDTYP_DATETIME; break;
    case SW_SERVICE_FIELDTYPE_USER: nRet = PROPERTY_MAP_FLDTYP_USER; break;
    case SW_SERVICE_FIELDTYPE_SET_EXP: nRet = PROPERTY_MAP_FLDTYP_SET_EXP; break;
    case SW_SERVICE_FIELDTYPE_GET_EXP: nRet = PROPERTY_MAP_FLDTYP_GET_EXP; break;
    case SW_SERVICE_FIELDTYPE_FILE_NAME: nRet = PROPERTY_MAP_FLDTYP_FILE_NAME; break;
    case SW_SERVICE_FIELDTYPE_PAGE_NUM: nRet = PROPERTY_MAP_FLDTYP_PAGE_NUM; break;
    case SW_SERVICE_FIELDTYPE_AUTHOR: nRet = PROPERTY_MAP_FLDTYP_AUTHOR; break;
    case SW_SERVICE_FIELDTYPE_CHAPTER: nRet = PROPERTY_MAP_FLDTYP_CHAPTER; break;
    case SW_SERVICE_FIELDTYPE_GET_REFERENCE: nRet = PROPERTY_MAP_FLDTYP_GET_REFERENCE; break;
    case SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT: nRet = PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT; break;
    case SW_SERVICE_FIELDTYPE_ANNOTATION: nRet = PROPERTY_MAP_FLDTYP_ANNOTATION; break;
    case SW_SERVICE_FIELDTYPE_INPUT_USER:
    case SW_SERVICE_FIELDTYPE_INPUT: nRet = PROPERTY_MAP_FLDTYP_INPUT; break;
    case SW_SERVICE_FIELDTYPE_MACRO: nRet = PROPERTY_MAP_FLDTYP_MACRO; break;
    case SW_SERVICE_FIELDTYPE_DDE: nRet = PROPERTY_MAP_FLDTYP_DDE; break;
    case SW_SERVICE_FIELDTYPE_HIDDEN_PARA: nRet = PROPERTY_MAP_FLDTYP_HIDDEN_PARA; break;
    case SW_SERVICE_FIELDTYPE_DOC_INFO: nRet = PROPERTY_MAP_FLDTYP_DOC_INFO; break;
    case SW_SERVICE_FIELDTYPE_TEMPLATE_NAME: nRet = PROPERTY_MAP_FLDTYP_TEMPLATE_NAME; break;
    case SW_SERVICE_FIELDTYPE_USER_EXT: nRet = PROPERTY_MAP_FLDTYP_USER_EXT; break;
    case SW_SERVICE_FIELDTYPE_REF_PAGE_SET: nRet = PROPERTY_MAP_FLDTYP_REF_PAGE_SET; break;
    case SW_SERVICE_FIELDTYPE_REF_PAGE_GET: nRet = PROPERTY_MAP_FLDTYP_REF_PAGE_GET; break;
    case SW_SERVICE_FIELDTYPE_JUMP_EDIT: nRet = PROPERTY_MAP_FLDTYP_JUMP_EDIT; break;
    case SW_SERVICE_FIELDTYPE_SCRIPT: nRet = PROPERTY_MAP_FLDTYP_SCRIPT; break;
    case SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET: nRet = PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET; break;
    case SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET: nRet = PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET; break;
    case SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM: nRet = PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM; break;
    case SW_SERVICE_FIELDTYPE_DATABASE: nRet = PROPERTY_MAP_FLDTYP_DATABASE; break;
    case SW_SERVICE_FIELDTYPE_DATABASE_NAME: nRet = PROPERTY_MAP_FLDTYP_DATABASE_NAME; break;
    case SW_SERVICE_FIELDTYPE_TABLE_FORMULA: nRet = PROPERTY_MAP_FLDTYP_TABLE_FORMULA; break;
    case SW_SERVICE_FIELDTYPE_PAGE_COUNT:
    case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT:
    case SW_SERVICE_FIELDTYPE_WORD_COUNT:
    case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT:
    case SW_SERVICE_FIELDTYPE_TABLE_COUNT:
    case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT:
    case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT: nRet = PROPERTY_MAP_FLDTYP_DOCSTAT; break;
    case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR:
    case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR:
    case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR; break;
    case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME:
    case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME:
    case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME; break;
    case SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME; break;
    case SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM; break;
    case SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION:
    case SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS:
    case SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT:
    case SW_SERVICE_FIELDTYPE_DOCINFO_TITLE: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_MISC; break;
    case SW_SERVICE_FIELDTYPE_DOCINFO_REVISION: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_REVISION; break;
    case SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY: nRet = PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY; break;
    case SW_SERVICE_FIELDTYPE_DUMMY_0:
    case SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS: nRet = PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS; break;
    case SW_SERVICE_FIELDTYPE_DROPDOWN: nRet = PROPERTY_MAP_FLDTYP_DROPDOWN; break;
    case SW_SERVICE_FIELDTYPE_DUMMY_4:
    case SW_SERVICE_FIELDTYPE_DUMMY_5:
    case SW_SERVICE_FIELDTYPE_DUMMY_6:
    case SW_SERVICE_FIELDTYPE_DUMMY_7:
                nRet = PROPERTY_MAP_FLDTYP_DUMMY_0; break;
    case SW_SERVICE_FIELDMASTER_USER: nRet = PROPERTY_MAP_FLDMSTR_USER; break;
    case SW_SERVICE_FIELDMASTER_DDE: nRet = PROPERTY_MAP_FLDMSTR_DDE; break;
    case SW_SERVICE_FIELDMASTER_SET_EXP: nRet = PROPERTY_MAP_FLDMSTR_SET_EXP; break;
    case SW_SERVICE_FIELDMASTER_DATABASE: nRet = PROPERTY_MAP_FLDMSTR_DATABASE; break;
    case SW_SERVICE_FIELDMASTER_BIBLIOGRAPHY: nRet = PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY; break;
    case SW_SERVICE_FIELDMASTER_DUMMY2:
    case SW_SERVICE_FIELDMASTER_DUMMY3:
    case SW_SERVICE_FIELDMASTER_DUMMY4:
    case SW_SERVICE_FIELDMASTER_DUMMY5: nRet = PROPERTY_MAP_FLDMSTR_DUMMY0; break;
    case SW_SERVICE_FIELDTYPE_HIDDEN_TEXT: nRet = PROPERTY_MAP_FLDTYP_HIDDEN_TEXT; break;
    default:
        OSL_FAIL( "wrong service id" );
        nRet = USHRT_MAX;
    }
    return nRet;
}

/******************************************************************
 * SwXFieldMaster
 ******************************************************************/

class SwXFieldMaster::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper
    SwXFieldMaster & m_rThis;

public:
    ::cppu::OInterfaceContainerHelper m_EventListeners;

    SwDoc*          m_pDoc;

    bool            m_bIsDescriptor;

    sal_uInt16      m_nResTypeId;

    OUString        m_sParam1;  // Content / Database / NumberingSeparator
    OUString        m_sParam2;  // -    /DataTablename
    OUString        m_sParam3;  // -    /DataFieldName
    OUString        m_sParam4;
    OUString        m_sParam5;  // -    /DataBaseURL
    OUString        m_sParam6;  // -    /DataBaseResource
    double          m_fParam1;  // Value / -
    sal_Int8        m_nParam1;  // ChapterNumberingLevel
    sal_Bool        m_bParam1;  // IsExpression
    sal_Int32       m_nParam2;

    Impl(SwXFieldMaster & rThis, SwModify *const pModify,
            SwDoc & rDoc, sal_uInt16 const nResId, bool const bIsDescriptor)
        : SwClient(pModify)
        , m_rThis(rThis)
        , m_EventListeners(m_Mutex)
        , m_pDoc(& rDoc)
        , m_bIsDescriptor(bIsDescriptor)
        , m_nResTypeId(nResId)
        , m_fParam1(0.0)
        , m_nParam1(-1)
        , m_bParam1(sal_False)
        , m_nParam2(0)
    { }

protected:
    // SwClient
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew);
};

namespace
{
    class theSwXFieldMasterUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXFieldMasterUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXFieldMaster::getUnoTunnelId()
{
    return theSwXFieldMasterUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXFieldMaster::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXFieldMaster>(rId, this);
}

OUString SAL_CALL
SwXFieldMaster::getImplementationName()
throw (uno::RuntimeException)
{
    return OUString("SwXFieldMaster");
}

sal_Bool SAL_CALL
SwXFieldMaster::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    sal_Bool bRet = sal_False;
    if(rServiceName.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextFieldMaster")))
        bRet = sal_True;
    else
    {
        const sal_Char* pEntry;
        switch (m_pImpl->m_nResTypeId)
        {
        case RES_USERFLD:   pEntry = "User";            break;
        case RES_DBFLD:     pEntry = "Database";        break;
        case RES_SETEXPFLD: pEntry = "SetExpression";   break;
        case RES_DDEFLD:    pEntry = "DDE";             break;
        case RES_AUTHORITY: pEntry = "Bibliography";    break;
        default: pEntry = 0;
        }
        if( pEntry )
        {
            OString aTmp = OStringBuffer(RTL_CONSTASCII_STRINGPARAM(
                "com.sun.star.text.fieldmaster.")).append(pEntry).
                makeStringAndClear();
            bRet = rServiceName.equalsAsciiL(aTmp.getStr(), aTmp.getLength());
        }
    }
    return bRet;
}

uno::Sequence< OUString > SAL_CALL
SwXFieldMaster::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextFieldMaster";

    const sal_Char* pEntry1;
    switch (m_pImpl->m_nResTypeId)
    {
    case RES_USERFLD:   pEntry1 = "User";           break;
    case RES_DBFLD:     pEntry1 = "Database";       break;
    case RES_SETEXPFLD: pEntry1 = "SetExpression";  break;
    case RES_DDEFLD:    pEntry1 = "DDE";            break;
    case RES_AUTHORITY: pEntry1 = "Bibliography";   break;
    default: pEntry1 = 0;
    }
    if( pEntry1 )
    {
        String s;
        s.AppendAscii( "com.sun.star.text.fieldmaster." ).AppendAscii( pEntry1 );
        pArray[1] = s;
    }
    return aRet;
}

SwXFieldMaster::SwXFieldMaster(SwDoc *const pDoc, sal_uInt16 const nResId)
    : m_pImpl(new Impl(*this, pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD),
                *pDoc, nResId, true))
{
}

SwXFieldMaster::SwXFieldMaster(SwFieldType& rType, SwDoc & rDoc)
    : m_pImpl(new Impl(*this, &rType, rDoc, rType.Which(), false))
{
}

SwXFieldMaster::~SwXFieldMaster()
{
}

uno::Reference<beans::XPropertySet>
SwXFieldMaster::CreateXFieldMaster(SwDoc & rDoc, SwFieldType & rType)
{
    // re-use existing SwXFieldMaster
    uno::Reference<beans::XPropertySet> xFM(rType.GetXObject());
    if (!xFM.is())
    {
        SwXFieldMaster *const pFM(new SwXFieldMaster(rType, rDoc));
        xFM.set(pFM);
        rType.SetXObject(xFM);
    }
    return xFM;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL
SwXFieldMaster::getPropertySetInfo()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference< beans::XPropertySetInfo >  aRef =
                        aSwMapProvider.GetPropertySet(
            lcl_GetPropMapIdForFieldType(m_pImpl->m_nResTypeId))->getPropertySetInfo();
    return aRef;
}

void SAL_CALL SwXFieldMaster::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
       lang::IllegalArgumentException, lang::WrappedTargetException,
       uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFieldType* pType = GetFldType(true);
    if(pType)
    {
        bool bSetValue = true;
        if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_SUB_TYPE)))
        {
            const ::std::vector<OUString>& rExtraArr(
                    SwStyleNameMapper::GetExtraUINameArray());
            OUString sTypeName = pType->GetName();
            static sal_uInt16 nIds[] =
            {
                RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
                0
            };
            for(const sal_uInt16 * pIds = nIds; *pIds; ++pIds)
            {
                if(sTypeName == rExtraArr[ *pIds ] )
                {
                    bSetValue = false;
                    break;
                }
            }
        }
        if( bSetValue )
        {
            // nothing special to be done here for the properties
            // UNO_NAME_DATA_BASE_NAME and UNO_NAME_DATA_BASE_URL.
            // We just call PutValue (empty string is allowed).
            // Thus the last property set will be used as Data Source.

            sal_uInt16 nMId = GetFieldTypeMId( rPropertyName, *pType  );
            if( USHRT_MAX != nMId )
                pType->PutValue( rValue, nMId );
            else
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
    else if (!pType && m_pImpl->m_pDoc &&
        (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NAME))))
    {
        OUString uTmp;
        rValue >>= uTmp;
        String sTypeName(uTmp);
        SwFieldType * pType2 = m_pImpl->m_pDoc->GetFldType(
                m_pImpl->m_nResTypeId, sTypeName, sal_False);

        String sTable(SW_RES(STR_POOLCOLL_LABEL_TABLE));
        String sDrawing(SW_RES(STR_POOLCOLL_LABEL_DRAWING));
        String sFrame(SW_RES(STR_POOLCOLL_LABEL_FRAME));
        String sIllustration(SW_RES(STR_POOLCOLL_LABEL_ABB));

        if(pType2 ||
            (RES_SETEXPFLD == m_pImpl->m_nResTypeId &&
            ( sTypeName == sTable || sTypeName == sDrawing ||
              sTypeName == sFrame || sTypeName == sIllustration )))
        {
            throw lang::IllegalArgumentException();
        }

        switch (m_pImpl->m_nResTypeId)
        {
            case RES_USERFLD :
            {
                SwUserFieldType aType(m_pImpl->m_pDoc, sTypeName);
                pType2 = m_pImpl->m_pDoc->InsertFldType(aType);
                static_cast<SwUserFieldType*>(pType2)->SetContent(m_pImpl->m_sParam1);
                static_cast<SwUserFieldType*>(pType2)->SetValue(m_pImpl->m_fParam1);
                static_cast<SwUserFieldType*>(pType2)->SetType(m_pImpl->m_bParam1
                    ? nsSwGetSetExpType::GSE_EXPR : nsSwGetSetExpType::GSE_STRING);
            }
            break;
            case RES_DDEFLD :
            {
                SwDDEFieldType aType(sTypeName, m_pImpl->m_sParam1,
                    sal::static_int_cast<sal_uInt16>((m_pImpl->m_bParam1)
                        ? sfx2::LINKUPDATE_ALWAYS : sfx2::LINKUPDATE_ONCALL));
                pType2 = m_pImpl->m_pDoc->InsertFldType(aType);
            }
            break;
            case RES_SETEXPFLD :
            {
                SwSetExpFieldType aType(m_pImpl->m_pDoc, sTypeName);
                if (!m_pImpl->m_sParam1.isEmpty())
                    aType.SetDelimiter(OUString(m_pImpl->m_sParam1[0]));
                if (m_pImpl->m_nParam1 > -1 && m_pImpl->m_nParam1 < MAXLEVEL)
                    aType.SetOutlineLvl(m_pImpl->m_nParam1);
                pType2 = m_pImpl->m_pDoc->InsertFldType(aType);
            }
            break;
            case RES_DBFLD :
            {
                rValue >>= m_pImpl->m_sParam3;
                pType = GetFldType();
            }
            break;
        }
        if (!pType2)
        {
            throw uno::RuntimeException("no field type found!", *this);
        }
        pType2->Add(m_pImpl.get());
        m_pImpl->m_bIsDescriptor = false;
    }
    else
    {
        switch (m_pImpl->m_nResTypeId)
        {
        case RES_USERFLD:
            if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CONTENT)))
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_VALUE )))
            {
                if(rValue.getValueType() != ::getCppuType(static_cast<const double*>(0)))
                    throw lang::IllegalArgumentException();
                rValue >>= m_pImpl->m_fParam1;
            }
            else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_EXPRESSION )))
            {
                if(rValue.getValueType() != ::getBooleanCppuType())
                    throw lang::IllegalArgumentException();
                rValue >>= m_pImpl->m_bParam1;
            }

            break;
        case RES_DBFLD:
            if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME)))
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME)))
                rValue >>= m_pImpl->m_sParam2;
            else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_COLUMN_NAME)))
                rValue >>= m_pImpl->m_sParam3;
            else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_COMMAND_TYPE)))
                rValue >>= m_pImpl->m_nParam2;
            if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_URL)))
                rValue >>= m_pImpl->m_sParam5;

            if (  (   !m_pImpl->m_sParam1.isEmpty()
                   || !m_pImpl->m_sParam5.isEmpty())
                && !m_pImpl->m_sParam2.isEmpty()
                && !m_pImpl->m_sParam3.isEmpty())
            {
                GetFldType();
            }
            break;
        case  RES_SETEXPFLD:
            if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NUMBERING_SEPARATOR)))
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAPTER_NUMBERING_LEVEL)))
                rValue >>= m_pImpl->m_nParam1;
            break;
        case RES_DDEFLD:
            {
                sal_uInt16 nPart = rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DDE_COMMAND_TYPE))  ? 0 :
                        rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DDE_COMMAND_FILE))  ? 1 :
                        rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DDE_COMMAND_ELEMENT))  ? 2 :
                        rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC_UPDATE)) ? 3 : USHRT_MAX;
                if(nPart  < 3 )
                {
                    if (m_pImpl->m_sParam1.isEmpty())
                    {
                        m_pImpl->m_sParam1 = OUString(sfx2::cTokenSeparator)
                                + OUString(sfx2::cTokenSeparator);
                    }
                    OUString sTmp;
                    rValue >>= sTmp;
                    sal_Int32 nIndex(0);
                    sal_Int32 nStart(0);
                    while (nIndex < m_pImpl->m_sParam1.getLength())
                    {
                        if (m_pImpl->m_sParam1[nIndex] == sfx2::cTokenSeparator)
                        {
                            if (0 == nPart)
                                break;
                            nStart = nIndex + 1;
                            --nPart;
                        }
                        ++nIndex;
                    }
                    assert(0 == nPart);
                    m_pImpl->m_sParam1 = m_pImpl->m_sParam1.replaceAt(
                            nStart, nIndex - nStart, sTmp);
                }
                else if(3 == nPart)
                {
                    rValue >>= m_pImpl->m_bParam1;
                }
            }
            break;
        default:
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
}

SwFieldType* SwXFieldMaster::GetFldType(bool const bDontCreate) const
{
    if (!bDontCreate && RES_DBFLD == m_pImpl->m_nResTypeId
        && m_pImpl->m_bIsDescriptor && m_pImpl->m_pDoc)
    {
        SwDBData aData;

        // set DataSource
        svx::ODataAccessDescriptor aAcc;
        if (!m_pImpl->m_sParam1.isEmpty())
            aAcc[svx::daDataSource]        <<= m_pImpl->m_sParam1; // DataBaseName
        else if (!m_pImpl->m_sParam5.isEmpty())
            aAcc[svx::daDatabaseLocation]  <<= m_pImpl->m_sParam5; // DataBaseURL
        aData.sDataSource = aAcc.getDataSource();

        aData.sCommand = m_pImpl->m_sParam2;
        aData.nCommandType = m_pImpl->m_nParam2;
        SwDBFieldType aType(m_pImpl->m_pDoc, m_pImpl->m_sParam3, aData);
        SwFieldType *const pType = m_pImpl->m_pDoc->InsertFldType(aType);
        pType->Add(m_pImpl.get());
        const_cast<SwXFieldMaster*>(this)->m_pImpl->m_bIsDescriptor = false;
    }
    if (m_pImpl->m_bIsDescriptor)
        return 0;
    else
        return static_cast<SwFieldType*>(const_cast<SwModify*>(m_pImpl->GetRegisteredIn()));
}

typedef std::vector<SwFmtFld*> SwDependentFields;

uno::Any SAL_CALL
SwXFieldMaster::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFieldType* pType = GetFldType(true);
    if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_INSTANCE_NAME)) )
    {
        OUString sName;
        if(pType)
            SwXTextFieldMasters::getInstanceName(*pType, sName);
        aRet <<= sName;
    }
    else if(pType)
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NAME) ))
        {
            aRet <<= SwXFieldMaster::GetProgrammaticName(*pType, *m_pImpl->m_pDoc);
        }
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS)) )
        {
            //fill all text fields into a sequence
            SwDependentFields aFldArr;
            SwIterator<SwFmtFld,SwFieldType> aIter( *pType );
            SwFmtFld* pFld = aIter.First();
            while(pFld)
            {
                if(pFld->IsFldInDoc())
                    aFldArr.push_back(pFld);
                pFld = aIter.Next();
            }

            uno::Sequence<uno::Reference <text::XDependentTextField> > aRetSeq(aFldArr.size());
            uno::Reference<text::XDependentTextField>* pRetSeq = aRetSeq.getArray();
            for(sal_uInt16 i = 0; i < aFldArr.size(); i++)
            {
                pFld = aFldArr[i];
                uno::Reference<text::XTextField> const xField =
                    SwXTextField::CreateXTextField(*m_pImpl->m_pDoc, *pFld);

                pRetSeq[i] = uno::Reference<text::XDependentTextField>(xField,
                        uno::UNO_QUERY);
            }
            aRet <<= aRetSeq;
        }
        else if(pType)
        {
            //TODO: Properties fuer die uebrigen Feldtypen einbauen
            sal_uInt16 nMId = GetFieldTypeMId( rPropertyName, *pType );
            if (USHRT_MAX == nMId)
            {
                throw beans::UnknownPropertyException(
                        "Unknown property: " + rPropertyName,
                        static_cast<cppu::OWeakObject *>(this));
            }
            pType->QueryValue( aRet, nMId );

            if (rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME)) ||
                rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_DATA_BASE_URL)))
            {
                OUString aDataSource;
                aRet >>= aDataSource;
                aRet <<= OUString();

                OUString *pStr = 0;     // only one of this properties will return
                                        // a non-empty string.
                INetURLObject aObj;
                aObj.SetURL( aDataSource );
                bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
                if (bIsURL && rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_DATA_BASE_URL)))
                    pStr = &aDataSource;        // DataBaseURL
                else if (!bIsURL && rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME)))
                    pStr = &aDataSource;        // DataBaseName

                if (pStr)
                    aRet <<= *pStr;
            }
        }
        else
        {
            if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_COMMAND_TYPE)) )
                aRet <<= m_pImpl->m_nParam2;
        }
    }
    else
    {
        if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_COMMAND_TYPE)) )
            aRet <<= m_pImpl->m_nParam2;
        else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DEPENDENT_TEXT_FIELDS)) )
        {
            uno::Sequence<uno::Reference <text::XDependentTextField> > aRetSeq(0);
            aRet <<= aRetSeq;
        }
        else
        {
            switch (m_pImpl->m_nResTypeId)
            {
            case RES_USERFLD:
                if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CONTENT)) )
                    aRet <<= m_pImpl->m_sParam1;
                else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_VALUE )))
                    aRet <<= m_pImpl->m_fParam1;
                else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_EXPRESSION )))
                    aRet.setValue(&m_pImpl->m_bParam1, ::getBooleanCppuType());
                break;
            case RES_DBFLD:
                if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME)) ||
                   rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_URL)))
                {
                     // only one of these properties returns a non-empty string.
                    INetURLObject aObj;
                    aObj.SetURL(m_pImpl->m_sParam5); // SetSmartURL
                    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
                    if (bIsURL && rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_URL)))
                        aRet <<= m_pImpl->m_sParam5; // DataBaseURL
                    else if ( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME)))
                        aRet <<= m_pImpl->m_sParam1; // DataBaseName
                }
                else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME)))
                    aRet <<= m_pImpl->m_sParam2;
                else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_COLUMN_NAME)))
                    aRet <<= m_pImpl->m_sParam3;
                break;
            case RES_SETEXPFLD:
                if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NUMBERING_SEPARATOR)))
                    aRet <<= m_pImpl->m_sParam1;
                else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAPTER_NUMBERING_LEVEL)))
                    aRet <<= m_pImpl->m_nParam1;
                break;
            case RES_DDEFLD:
                {
                    sal_uInt16 nPart = rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DDE_COMMAND_TYPE))  ? 0 :
                        rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DDE_COMMAND_FILE)) ? 1 :
                            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DDE_COMMAND_ELEMENT))  ? 2 :
                            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC_UPDATE)) ? 3 : USHRT_MAX;
                    if(nPart  < 3 )
                        aRet <<= m_pImpl->m_sParam1.getToken(nPart, sfx2::cTokenSeparator);
                    else if(3 == nPart)
                        aRet.setValue(&m_pImpl->m_bParam1, ::getBooleanCppuType());
                }
                break;
            default:
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            }
        }
    }
    return aRet;
}

void SwXFieldMaster::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXFieldMaster::dispose()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFieldType *const pFldType = GetFldType(true);
    if (!pFldType)
        throw uno::RuntimeException();
    sal_uInt16 nTypeIdx = USHRT_MAX;
    const SwFldTypes* pTypes = m_pImpl->m_pDoc->GetFldTypes();
    for( sal_uInt16 i = 0; i < pTypes->size(); i++ )
    {
        if((*pTypes)[i] == pFldType)
            nTypeIdx = i;
    }

    // zuerst alle Felder loeschen
    SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
    SwFmtFld* pFld = aIter.First();
    while(pFld)
    {
        // Feld im Undo?
        SwTxtFld *pTxtFld = pFld->GetTxtFld();
        if(pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
        {
            SwTxtNode& rTxtNode = (SwTxtNode&)*pTxtFld->GetpTxtNode();
            SwPaM aPam(rTxtNode, *pTxtFld->GetStart());
            aPam.SetMark();
            aPam.Move();
            m_pImpl->m_pDoc->DeleteAndJoin(aPam);
        }
        pFld = aIter.Next();
    }
    // dann den FieldType loeschen
    m_pImpl->m_pDoc->RemoveFldType(nTypeIdx);
}

void SAL_CALL SwXFieldMaster::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXFieldMaster::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

void SwXFieldMaster::Impl::Modify(
        SfxPoolItem const*const pOld, SfxPoolItem const*const pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_pDoc = 0;
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(m_rThis));
        m_EventListeners.disposeAndClear(ev);
    }
}

OUString SwXFieldMaster::GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc)
{
    OUString sRet(rType.GetName());
    if(RES_SETEXPFLD == rType.Which())
    {
        const SwFldTypes* pTypes = rDoc.GetFldTypes();
        for( sal_uInt16 i = 0; i <= INIT_FLDTYPES; i++ )
        {
            if((*pTypes)[i] == &rType)
            {
                sRet = SwStyleNameMapper::GetProgName ( sRet, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                break;
            }
        }
    }
    return sRet;
}

OUString SwXFieldMaster::LocalizeFormula(
    const SwSetExpField& rFld,
    const OUString& rFormula,
    sal_Bool bQuery)
{
    const OUString sTypeName(rFld.GetTyp()->GetName());
    OUString sProgName = SwStyleNameMapper::GetProgName(sTypeName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
    if(sProgName != sTypeName)
    {
        OUString sSource = bQuery ? sTypeName : sProgName;
        OUString sDest = bQuery ? sProgName : sTypeName;
        if(rFormula.startsWith(sSource))
        {
            OUString sTmpFormula = sDest;
            sTmpFormula += rFormula.copy(sSource.getLength());
            return sTmpFormula;
        }
    }
    return rFormula;
}

/******************************************************************
 * SwXTextField
 ******************************************************************/

uno::Reference<text::XTextField>
SwXTextField::CreateXTextField(SwDoc & rDoc, SwFmtFld const& rFmt)
{
    // re-use existing SwXTextField
    uno::Reference<text::XTextField> xField(rFmt.GetXTextField());
    if (!xField.is())
    {
        SwXTextField *const pField(new SwXTextField(rFmt, rDoc));
        xField.set(pField);
        const_cast<SwFmtFld &>(rFmt).SetXTextField(xField);
    }
    return xField;
}

struct SwFieldProperties_Impl
{
    OUString    sPar1;
    OUString    sPar2;
    OUString    sPar3;
    OUString    sPar4;
    OUString    sPar5;
    OUString    sPar6;
    Date            aDate;
    double          fDouble;
    uno::Sequence<beans::PropertyValue> aPropSeq;
    uno::Sequence<OUString> aStrings;
    util::DateTime* pDateTime;

    sal_Int32       nSubType;
    sal_Int32       nFormat;
    sal_uInt16      nUSHORT1;
    sal_uInt16      nUSHORT2;
    sal_Int16       nSHORT1;
    sal_Int8        nByte1;
    bool            bFormatIsDefault;
    sal_Bool        bBool1;
    sal_Bool        bBool2;
    sal_Bool        bBool3;
    sal_Bool        bBool4;

    SwFieldProperties_Impl():
        aDate( Date::EMPTY ),
        fDouble(0.),
        pDateTime(0),
        nSubType(0),
        nFormat(0),
        nUSHORT1(0),
        nUSHORT2(0),
        nSHORT1(0),
        nByte1(0),
        bFormatIsDefault(true),
        bBool1(sal_False),
        bBool2(sal_False),
        bBool3(sal_False),
        bBool4(sal_True) //Automatic language
        {}
    ~SwFieldProperties_Impl()
        {delete pDateTime;}

};

class SwXTextField::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper
    SwXTextField & m_rThis;

public:
    ::cppu::OInterfaceContainerHelper m_EventListeners;

    SwFmtFld const*     m_pFmtFld;
    SwDoc *             m_pDoc;
    SwTextAPIObject *   m_pTextObject;

    bool                m_bIsDescriptor;
    // required to access field master of not yet inserted fields
    SwClient            m_FieldTypeClient;
    bool                m_bCallUpdate;
    sal_uInt16          m_nServiceId;
    OUString            m_sTypeName;
    boost::scoped_ptr<SwFieldProperties_Impl> m_pProps;

    Impl(SwXTextField & rThis, SwDoc *const pDoc, SwFmtFld const*const pFmt,
            sal_uInt16 const nServiceId)
        : SwClient((pFmt) ? pDoc->GetUnoCallBack() : 0)
        , m_rThis(rThis)
        , m_EventListeners(m_Mutex)
        , m_pFmtFld(pFmt)
        , m_pDoc(pDoc)
        , m_pTextObject(0)
        , m_bIsDescriptor(pFmt == 0)
        , m_bCallUpdate(false)
        , m_nServiceId((pFmt)
                ? lcl_GetServiceForField(*pFmt->GetFld())
                : nServiceId)
        , m_pProps((pFmt) ? 0 : new SwFieldProperties_Impl)
    { }

    ~Impl()
    {
        if (m_pTextObject)
        {
            m_pTextObject->DisposeEditSource();
            m_pTextObject->release();
        }
    }

    void Invalidate();

    const SwField*      GetField() const;

protected:
    // SwClient
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew);
};

namespace
{
    class theSwXTextFieldUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextFieldUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextField::getUnoTunnelId()
{
    return theSwXTextFieldUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL
SwXTextField::getSomething(const uno::Sequence< sal_Int8 >& rId)
throw (uno::RuntimeException)
{
    return ::sw::UnoTunnelImpl<SwXTextField>(rId, this);
}

SwXTextField::SwXTextField(sal_uInt16 nServiceId, SwDoc* pDoc)
    : m_pImpl(new Impl(*this, pDoc, 0, nServiceId))
{
    //Set visible as default!
    if(SW_SERVICE_FIELDTYPE_SET_EXP == nServiceId ||
            SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM == nServiceId ||
            SW_SERVICE_FIELDTYPE_DATABASE == nServiceId ||
            SW_SERVICE_FIELDTYPE_DATABASE_NAME == nServiceId  )
        m_pImpl->m_pProps->bBool2 = sal_True;
    else if(SW_SERVICE_FIELDTYPE_TABLE_FORMULA == nServiceId)
        m_pImpl->m_pProps->bBool1 = sal_True;
    if(SW_SERVICE_FIELDTYPE_SET_EXP == nServiceId)
        m_pImpl->m_pProps->nUSHORT2 = USHRT_MAX;
}

SwXTextField::SwXTextField(const SwFmtFld& rFmt, SwDoc & rDoc)
    : m_pImpl(new Impl(*this, &rDoc, &rFmt, USHRT_MAX))
{
}

SwXTextField::~SwXTextField()
{
}

sal_uInt16 SwXTextField::GetServiceId() const
{
    return m_pImpl->m_nServiceId;
}

void SAL_CALL SwXTextField::attachTextFieldMaster(
        const uno::Reference< beans::XPropertySet > & xFieldMaster)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference< lang::XUnoTunnel > xMasterTunnel(xFieldMaster, uno::UNO_QUERY);
    if (!xMasterTunnel.is())
        throw lang::IllegalArgumentException();
    SwXFieldMaster* pMaster = reinterpret_cast< SwXFieldMaster * >(
            sal::static_int_cast< sal_IntPtr >( xMasterTunnel->getSomething( SwXFieldMaster::getUnoTunnelId()) ));

    SwFieldType* pFieldType = pMaster ? pMaster->GetFldType() : 0;
    if (!pFieldType ||
        pFieldType->Which() != lcl_ServiceIdToResId(m_pImpl->m_nServiceId))
    {
        throw lang::IllegalArgumentException();
    }
    m_pImpl->m_sTypeName = pFieldType->GetName();
    pFieldType->Add( &m_pImpl->m_FieldTypeClient );
}

uno::Reference< beans::XPropertySet > SAL_CALL
SwXTextField::getTextFieldMaster() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFieldType* pType = 0;
    if (m_pImpl->m_bIsDescriptor && m_pImpl->m_FieldTypeClient.GetRegisteredIn())
    {
        pType = static_cast<SwFieldType*>(const_cast<SwModify*>(
                    m_pImpl->m_FieldTypeClient.GetRegisteredIn()));
    }
    else
    {
        if (!m_pImpl->GetRegisteredIn())
            throw uno::RuntimeException();
        pType = m_pImpl->m_pFmtFld->GetFld()->GetTyp();
    }

    uno::Reference<beans::XPropertySet> const xRet(
            SwXFieldMaster::CreateXFieldMaster(*m_pImpl->m_pDoc, *pType));
    return xRet;
}

OUString SAL_CALL SwXTextField::getPresentation(sal_Bool bShowCommand)
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwField const*const pField = m_pImpl->GetField();
    if (!pField)
    {
        throw uno::RuntimeException();
    }
    OUString const ret( (bShowCommand)
            ? pField->GetFieldName()
            : pField->ExpandField(true) );
    return ret;
}

void SAL_CALL SwXTextField::attach(
        const uno::Reference< text::XTextRange > & xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (!m_pImpl->m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    //wurde ein FieldMaster attached, dann ist das Dokument schon festgelegt!
    if (!pDoc || (m_pImpl->m_pDoc && m_pImpl->m_pDoc != pDoc))
        throw lang::IllegalArgumentException();

    SwUnoInternalPaM aPam(*pDoc);
    //das muss jetzt sal_True liefern
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    SwField* pFld = 0;
    switch (m_pImpl->m_nServiceId)
    {
        case SW_SERVICE_FIELDTYPE_ANNOTATION:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_POSTITFLD);

            DateTime aDateTime( DateTime::EMPTY );
            if (m_pImpl->m_pProps->pDateTime)
            {
                aDateTime.SetYear(m_pImpl->m_pProps->pDateTime->Year);
                aDateTime.SetMonth(m_pImpl->m_pProps->pDateTime->Month);
                aDateTime.SetDay(m_pImpl->m_pProps->pDateTime->Day);
                aDateTime.SetHour(m_pImpl->m_pProps->pDateTime->Hours);
                aDateTime.SetMin(m_pImpl->m_pProps->pDateTime->Minutes);
                aDateTime.SetSec(m_pImpl->m_pProps->pDateTime->Seconds);
            }
            pFld = new SwPostItField((SwPostItFieldType*)pFldType,
                    m_pImpl->m_pProps->sPar1, m_pImpl->m_pProps->sPar2,
                    m_pImpl->m_pProps->sPar3, m_pImpl->m_pProps->sPar4,
                    aDateTime);
            if (m_pImpl->m_pTextObject)
            {
                SwPostItField *const pP(static_cast<SwPostItField *>(pFld));
                pP->SetTextObject(m_pImpl->m_pTextObject->CreateText());
                pP->SetPar2(m_pImpl->m_pTextObject->GetText());
            }
        }
        break;
        case SW_SERVICE_FIELDTYPE_SCRIPT:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_SCRIPTFLD);
            pFld = new SwScriptField((SwScriptFieldType*)pFldType,
                    m_pImpl->m_pProps->sPar1, m_pImpl->m_pProps->sPar2,
                    m_pImpl->m_pProps->bBool1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATETIME:
        {
            sal_uInt16 nSub = 0;
            if (m_pImpl->m_pProps->bBool1)
                nSub |= FIXEDFLD;
            if (m_pImpl->m_pProps->bBool2)
                nSub |= DATEFLD;
            else
                nSub |= TIMEFLD;
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_DATETIMEFLD);
            SwDateTimeField *const pDTField = new SwDateTimeField(
                    static_cast<SwDateTimeFieldType*>(pFldType),
                        nSub, m_pImpl->m_pProps->nFormat);
            pFld = pDTField;
            if (m_pImpl->m_pProps->fDouble > 0.)
            {
                pDTField->SetValue(m_pImpl->m_pProps->fDouble);
            }
            if (m_pImpl->m_pProps->pDateTime)
            {
                uno::Any aVal; aVal <<= *m_pImpl->m_pProps->pDateTime;
                pFld->PutValue( aVal, FIELD_PROP_DATE_TIME );
            }
            pDTField->SetOffset(m_pImpl->m_pProps->nSubType);
        }
        break;
        case SW_SERVICE_FIELDTYPE_FILE_NAME:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_FILENAMEFLD);
            sal_Int32 nFormat = m_pImpl->m_pProps->nFormat;
            if (m_pImpl->m_pProps->bBool2)
                nFormat |= FF_FIXED;
            SwFileNameField *const pFNField = new SwFileNameField(
                    static_cast<SwFileNameFieldType*>(pFldType), nFormat);
            pFld = pFNField;
            if (!m_pImpl->m_pProps->sPar3.isEmpty())
                pFNField->SetExpansion(m_pImpl->m_pProps->sPar3);
            uno::Any aFormat;
            aFormat <<= m_pImpl->m_pProps->nFormat;
            pFld->PutValue( aFormat, FIELD_PROP_FORMAT );
        }
        break;
        case SW_SERVICE_FIELDTYPE_TEMPLATE_NAME:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_TEMPLNAMEFLD);
            pFld = new SwTemplNameField((SwTemplNameFieldType*)pFldType,
                                        m_pImpl->m_pProps->nFormat);
            uno::Any aFormat;
            aFormat <<= m_pImpl->m_pProps->nFormat;
            pFld->PutValue(aFormat, FIELD_PROP_FORMAT);
        }
        break;
        case SW_SERVICE_FIELDTYPE_CHAPTER:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_CHAPTERFLD);
            SwChapterField *const pChapterField = new SwChapterField(
                    static_cast<SwChapterFieldType*>(pFldType),
                    m_pImpl->m_pProps->nUSHORT1);
            pFld = pChapterField;
            pChapterField->SetLevel(m_pImpl->m_pProps->nByte1);
            uno::Any aVal;
            aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
            pFld->PutValue(aVal, FIELD_PROP_USHORT1 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_AUTHOR:
        {
            long nFormat = m_pImpl->m_pProps->bBool1 ? AF_NAME : AF_SHORTCUT;
            if (m_pImpl->m_pProps->bBool2)
                nFormat |= AF_FIXED;

            SwFieldType* pFldType = pDoc->GetSysFldType(RES_AUTHORFLD);
            SwAuthorField *const pAuthorField = new SwAuthorField(
                    static_cast<SwAuthorFieldType*>(pFldType), nFormat);
            pFld = pAuthorField;
            pAuthorField->SetExpansion(m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT:
        case SW_SERVICE_FIELDTYPE_HIDDEN_TEXT:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_HIDDENTXTFLD);
            SwHiddenTxtField *const pHTField = new SwHiddenTxtField(
                    static_cast<SwHiddenTxtFieldType*>(pFldType),
                    m_pImpl->m_pProps->sPar1,
                    m_pImpl->m_pProps->sPar2, m_pImpl->m_pProps->sPar3,
                    static_cast<sal_uInt16>(SW_SERVICE_FIELDTYPE_HIDDEN_TEXT == m_pImpl->m_nServiceId ?
                         TYP_HIDDENTXTFLD : TYP_CONDTXTFLD));
            pFld = pHTField;
            pHTField->SetValue(m_pImpl->m_pProps->bBool1);
            uno::Any aVal;
            aVal <<= m_pImpl->m_pProps->sPar4;
            pFld->PutValue(aVal, FIELD_PROP_PAR4 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_HIDDEN_PARA:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_HIDDENPARAFLD);
            SwHiddenParaField *const pHPField = new SwHiddenParaField(
                    static_cast<SwHiddenParaFieldType*>(pFldType),
                    m_pImpl->m_pProps->sPar1);
            pFld = pHPField;
            pHPField->SetHidden(m_pImpl->m_pProps->bBool1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_GET_REFERENCE:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_GETREFFLD);
            pFld = new SwGetRefField((SwGetRefFieldType*)pFldType,
                        m_pImpl->m_pProps->sPar1,
                        0,
                        0,
                        0);
            if (!m_pImpl->m_pProps->sPar3.isEmpty())
                static_cast<SwGetRefField*>(pFld)->SetExpand(m_pImpl->m_pProps->sPar3);
            uno::Any aVal;
            aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
            pFld->PutValue(aVal, FIELD_PROP_USHORT1 );
            aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT2);
            pFld->PutValue(aVal, FIELD_PROP_USHORT2 );
            aVal <<= m_pImpl->m_pProps->nSHORT1;
            pFld->PutValue(aVal, FIELD_PROP_SHORT1 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_JUMP_EDIT:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_JUMPEDITFLD);
            pFld = new SwJumpEditField((SwJumpEditFieldType*)pFldType,
                    m_pImpl->m_pProps->nUSHORT1, m_pImpl->m_pProps->sPar2,
                    m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR     :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME  :
        case SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME         :
        case SW_SERVICE_FIELDTYPE_DOCINFO_DESCRIPTION       :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_AUTHOR     :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME  :
        case SW_SERVICE_FIELDTYPE_DOCINFO_CUSTOM            :
        case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_AUTHOR      :
        case SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME   :
        case SW_SERVICE_FIELDTYPE_DOCINFO_KEY_WORDS         :
        case SW_SERVICE_FIELDTYPE_DOCINFO_SUBJECT           :
        case SW_SERVICE_FIELDTYPE_DOCINFO_TITLE             :
        case SW_SERVICE_FIELDTYPE_DOCINFO_REVISION          :
        case SW_SERVICE_FIELDTYPE_DOC_INFO:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_DOCINFOFLD);
            sal_uInt16 nSubType = aDocInfoSubTypeFromService[
                    m_pImpl->m_nServiceId - SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_AUTHOR];
            if (SW_SERVICE_FIELDTYPE_DOCINFO_CHANGE_DATE_TIME == m_pImpl->m_nServiceId ||
                SW_SERVICE_FIELDTYPE_DOCINFO_CREATE_DATE_TIME == m_pImpl->m_nServiceId ||
                SW_SERVICE_FIELDTYPE_DOCINFO_PRINT_DATE_TIME == m_pImpl->m_nServiceId ||
                SW_SERVICE_FIELDTYPE_DOCINFO_EDIT_TIME == m_pImpl->m_nServiceId)
            {
                if (m_pImpl->m_pProps->bBool2) //IsDate
                {
                    nSubType &= 0xf0ff;
                    nSubType |= DI_SUB_DATE;
                }
                else
                {
                    nSubType &= 0xf0ff;
                    nSubType |= DI_SUB_TIME;
                }
            }
            if (m_pImpl->m_pProps->bBool1)
                nSubType |= DI_SUB_FIXED;
            pFld = new SwDocInfoField(
                    static_cast<SwDocInfoFieldType*>(pFldType), nSubType,
                    m_pImpl->m_pProps->sPar4, m_pImpl->m_pProps->nFormat);
            if (!m_pImpl->m_pProps->sPar3.isEmpty())
                static_cast<SwDocInfoField*>(pFld)->SetExpansion(m_pImpl->m_pProps->sPar3);
        }
        break;
        case SW_SERVICE_FIELDTYPE_USER_EXT:
        {
            sal_Int32 nFormat = 0;
            if (m_pImpl->m_pProps->bBool1)
                nFormat = AF_FIXED;

            SwFieldType* pFldType = pDoc->GetSysFldType(RES_EXTUSERFLD);
            SwExtUserField *const pEUField = new SwExtUserField(
                static_cast<SwExtUserFieldType*>(pFldType),
                m_pImpl->m_pProps->nUSHORT1, nFormat);
            pFld = pEUField;
            pEUField->SetExpansion(m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_USER:
        {
            SwFieldType* pFldType =
                pDoc->GetFldType(RES_USERFLD, m_pImpl->m_sTypeName, true);
            if (!pFldType)
                throw uno::RuntimeException();
            sal_uInt16 nUserSubType = (m_pImpl->m_pProps->bBool1)
                ? nsSwExtendedSubType::SUB_INVISIBLE : 0;
            if (m_pImpl->m_pProps->bBool2)
                nUserSubType |= nsSwExtendedSubType::SUB_CMD;
            if (m_pImpl->m_pProps->bFormatIsDefault &&
                nsSwGetSetExpType::GSE_STRING == ((SwUserFieldType*)pFldType)->GetType())
            {
                m_pImpl->m_pProps->nFormat = -1;
            }
            pFld = new SwUserField((SwUserFieldType*)pFldType,
                                nUserSubType,
                                m_pImpl->m_pProps->nFormat);
        }
        break;
        case SW_SERVICE_FIELDTYPE_REF_PAGE_SET:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_REFPAGESETFLD);
            pFld = new SwRefPageSetField( (SwRefPageSetFieldType*)pFldType,
                                m_pImpl->m_pProps->nUSHORT1,
                                m_pImpl->m_pProps->bBool1 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_REF_PAGE_GET:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_REFPAGEGETFLD);
            SwRefPageGetField *const pRGField = new SwRefPageGetField(
                    static_cast<SwRefPageGetFieldType*>(pFldType),
                    m_pImpl->m_pProps->nUSHORT1 );
            pFld = pRGField;
            pRGField->SetText(m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_PAGE_NUM:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_PAGENUMBERFLD);
            SwPageNumberField *const pPNField = new SwPageNumberField(
                static_cast<SwPageNumberFieldType*>(pFldType), PG_RANDOM,
                m_pImpl->m_pProps->nFormat,
                m_pImpl->m_pProps->nUSHORT1);
            pFld = pPNField;
            pPNField->SetUserString(m_pImpl->m_pProps->sPar1);
            uno::Any aVal;
            aVal <<= m_pImpl->m_pProps->nSubType;
            pFld->PutValue( aVal, FIELD_PROP_SUBTYPE );
        }
        break;
        case SW_SERVICE_FIELDTYPE_DDE:
        {
            SwFieldType* pFldType =
                pDoc->GetFldType(RES_DDEFLD, m_pImpl->m_sTypeName, true);
            if (!pFldType)
                throw uno::RuntimeException();
            pFld = new SwDDEField( (SwDDEFieldType*)pFldType );
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NAME:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_DBNAMEFLD);
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            pFld = new SwDBNameField((SwDBNameFieldType*)pFldType, aData);
            sal_uInt16  nSubType = pFld->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            pFld->SetSubType(nSubType);
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET:
        {
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_DBNEXTSETFLD);
            pFld = new SwDBNextSetField((SwDBNextSetFieldType*)pFldType,
                    m_pImpl->m_pProps->sPar3, aEmptyStr, aData);
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET:
        {
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            pFld = new SwDBNumSetField( (SwDBNumSetFieldType*)
                pDoc->GetSysFldType(RES_DBNUMSETFLD),
                m_pImpl->m_pProps->sPar3,
                OUString::number(m_pImpl->m_pProps->nFormat),
                aData );
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM:
        {
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            SwDBSetNumberField *const pDBSNField =
                new SwDBSetNumberField(static_cast<SwDBSetNumberFieldType*>(
                        pDoc->GetSysFldType(RES_DBSETNUMBERFLD)), aData,
                    m_pImpl->m_pProps->nUSHORT1);
            pFld = pDBSNField;
            pDBSNField->SetSetNumber(m_pImpl->m_pProps->nFormat);
            sal_uInt16  nSubType = pFld->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            pFld->SetSubType(nSubType);
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE:
        {
            SwFieldType* pFldType =
                pDoc->GetFldType(RES_DBFLD, m_pImpl->m_sTypeName, false);
            if (!pFldType)
                throw uno::RuntimeException();
            pFld = new SwDBField(static_cast<SwDBFieldType*>(pFldType),
                    m_pImpl->m_pProps->nFormat);
            ((SwDBField*)pFld)->InitContent(m_pImpl->m_pProps->sPar1);
            sal_uInt16  nSubType = pFld->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            pFld->SetSubType(nSubType);
        }
        break;
        case SW_SERVICE_FIELDTYPE_SET_EXP:
        {
            SwFieldType* pFldType =
                pDoc->GetFldType(RES_SETEXPFLD, m_pImpl->m_sTypeName, true);
            if (!pFldType)
                throw uno::RuntimeException();
            // detect the field type's sub type and set an appropriate number format
            if (m_pImpl->m_pProps->bFormatIsDefault &&
                nsSwGetSetExpType::GSE_STRING == ((SwSetExpFieldType*)pFldType)->GetType())
            {
                m_pImpl->m_pProps->nFormat = -1;
            }
            SwSetExpField *const pSEField = new SwSetExpField(
                static_cast<SwSetExpFieldType*>(pFldType),
                m_pImpl->m_pProps->sPar2,
                m_pImpl->m_pProps->nUSHORT2 != USHRT_MAX ?  //#i79471# the field can have a number format or a number_ing_ format
                m_pImpl->m_pProps->nUSHORT2 : m_pImpl->m_pProps->nFormat);
            pFld = pSEField;

            sal_uInt16  nSubType = pFld->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            if (m_pImpl->m_pProps->bBool3)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            else
                nSubType &= ~nsSwExtendedSubType::SUB_CMD;
            pFld->SetSubType(nSubType);
            pSEField->SetSeqNumber(m_pImpl->m_pProps->nUSHORT1);
            pSEField->SetInputFlag(m_pImpl->m_pProps->bBool1);
            pSEField->SetPromptText(m_pImpl->m_pProps->sPar3);
            if (!m_pImpl->m_pProps->sPar4.isEmpty())
                pSEField->ChgExpStr(m_pImpl->m_pProps->sPar4);

        }
        break;
        case SW_SERVICE_FIELDTYPE_GET_EXP:
        {
            sal_uInt16 nSubType;
            switch (m_pImpl->m_pProps->nSubType)
            {
                case text::SetVariableType::STRING: nSubType = nsSwGetSetExpType::GSE_STRING;   break;
                case text::SetVariableType::VAR:        nSubType = nsSwGetSetExpType::GSE_EXPR;  break;
                //case text::SetVariableType::SEQUENCE:   nSubType = nsSwGetSetExpType::GSE_SEQ;  break;
                case text::SetVariableType::FORMULA:    nSubType = nsSwGetSetExpType::GSE_FORMULA; break;
                default:
                    OSL_FAIL("wrong value");
                    nSubType = nsSwGetSetExpType::GSE_EXPR;
            }
            //make sure the SubType matches the field type
            SwFieldType* pSetExpFld = pDoc->GetFldType(
                    RES_SETEXPFLD, m_pImpl->m_pProps->sPar1, sal_False);
            bool bSetGetExpFieldUninitialized = false;
            if (pSetExpFld)
            {
                if (nSubType != nsSwGetSetExpType::GSE_STRING &&
                    static_cast< SwSetExpFieldType* >(pSetExpFld)->GetType() == nsSwGetSetExpType::GSE_STRING)
                nSubType = nsSwGetSetExpType::GSE_STRING;
            }
            else
                bSetGetExpFieldUninitialized = true; // #i82544#

            if (m_pImpl->m_pProps->bBool2)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            else
                nSubType &= ~nsSwExtendedSubType::SUB_CMD;
            SwGetExpField *const pGEField = new SwGetExpField(
                    static_cast<SwGetExpFieldType*>(
                        pDoc->GetSysFldType(RES_GETEXPFLD)),
                    m_pImpl->m_pProps->sPar1, nSubType,
                    m_pImpl->m_pProps->nFormat);
            pFld = pGEField;
            //TODO: SubType auswerten!
            if (!m_pImpl->m_pProps->sPar4.isEmpty())
                pGEField->ChgExpStr(m_pImpl->m_pProps->sPar4);
            // #i82544#
            if (bSetGetExpFieldUninitialized)
                pGEField->SetLateInitialization();
        }
        break;
        case SW_SERVICE_FIELDTYPE_INPUT_USER:
        case SW_SERVICE_FIELDTYPE_INPUT:
        {
            SwFieldType* pFldType =
                pDoc->GetFldType(RES_INPUTFLD, m_pImpl->m_sTypeName, true);
            if (!pFldType)
                throw uno::RuntimeException();
            sal_uInt16 nInpSubType =
                sal::static_int_cast<sal_uInt16>(
                    SW_SERVICE_FIELDTYPE_INPUT_USER == m_pImpl->m_nServiceId
                        ? INP_USR : INP_TXT);
            SwInputField * pTxtField =
                new SwInputField((SwInputFieldType*)pFldType,
                     m_pImpl->m_pProps->sPar1, m_pImpl->m_pProps->sPar2,
                                 nInpSubType);
            pTxtField->SetHelp(m_pImpl->m_pProps->sPar3);
            pTxtField->SetToolTip(m_pImpl->m_pProps->sPar4);

            pFld = pTxtField;
        }
        break;
        case SW_SERVICE_FIELDTYPE_MACRO:
        {
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_MACROFLD);
            OUString aName;

            // support for Scripting Framework macros
            if (!m_pImpl->m_pProps->sPar4.isEmpty())
            {
                aName = m_pImpl->m_pProps->sPar4;
            }
            else
            {
                SwMacroField::CreateMacroString(aName,
                    m_pImpl->m_pProps->sPar1, m_pImpl->m_pProps->sPar3);
            }
            pFld = new SwMacroField((SwMacroFieldType*)pFldType, aName,
                                    m_pImpl->m_pProps->sPar2);
        }
        break;
        case SW_SERVICE_FIELDTYPE_PAGE_COUNT            :
        case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT       :
        case SW_SERVICE_FIELDTYPE_WORD_COUNT            :
        case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT       :
        case SW_SERVICE_FIELDTYPE_TABLE_COUNT           :
        case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT  :
        case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT :
        {
            sal_uInt16 nSubType = DS_PAGE;
            switch (m_pImpl->m_nServiceId)
            {
//                  case SW_SERVICE_FIELDTYPE_PAGE_COUNT            : break;
                case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT       : nSubType = DS_PARA; break;
                case SW_SERVICE_FIELDTYPE_WORD_COUNT            : nSubType = DS_WORD; break;
                case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT       : nSubType = DS_CHAR; break;
                case SW_SERVICE_FIELDTYPE_TABLE_COUNT           : nSubType = DS_TBL;  break;
                case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT  : nSubType = DS_GRF;  break;
                case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT : nSubType = DS_OLE;  break;
            }
            SwFieldType* pFldType = pDoc->GetSysFldType(RES_DOCSTATFLD);
            pFld = new SwDocStatField(
                    static_cast<SwDocStatFieldType*>(pFldType),
                    nSubType, m_pImpl->m_pProps->nUSHORT2);
        }
        break;
        case SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY:
        {
            SwAuthorityFieldType const type(pDoc);
            pFld = new SwAuthorityField(static_cast<SwAuthorityFieldType*>(
                        pDoc->InsertFldType(type)),
                    aEmptyStr );
            if (m_pImpl->m_pProps->aPropSeq.getLength())
            {
                uno::Any aVal;
                aVal <<= m_pImpl->m_pProps->aPropSeq;
                pFld->PutValue( aVal, FIELD_PROP_PROP_SEQ );
            }
        }
        break;
        case SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS:
            // create field
            pFld = new SwCombinedCharField( (SwCombinedCharFieldType*)
                        pDoc->GetSysFldType(RES_COMBINED_CHARS),
                        m_pImpl->m_pProps->sPar1);
            break;
        case SW_SERVICE_FIELDTYPE_DROPDOWN:
        {
            SwDropDownField *const pDDField = new SwDropDownField(
                static_cast<SwDropDownFieldType *>(
                    pDoc->GetSysFldType(RES_DROPDOWN)));
            pFld = pDDField;

            pDDField->SetItems(m_pImpl->m_pProps->aStrings);
            pDDField->SetSelectedItem(m_pImpl->m_pProps->sPar1);
            pDDField->SetName(m_pImpl->m_pProps->sPar2);
            pDDField->SetHelp(m_pImpl->m_pProps->sPar3);
            pDDField->SetToolTip(m_pImpl->m_pProps->sPar4);
        }
        break;

        case SW_SERVICE_FIELDTYPE_TABLE_FORMULA :
        {
            // create field
            sal_uInt16 nType = nsSwGetSetExpType::GSE_FORMULA;
            if (m_pImpl->m_pProps->bBool1)
            {
                nType |= nsSwExtendedSubType::SUB_CMD;
                if (m_pImpl->m_pProps->bFormatIsDefault)
                    m_pImpl->m_pProps->nFormat = -1;
            }
            pFld = new SwTblField( (SwTblFieldType*)
                pDoc->GetSysFldType(RES_TABLEFLD),
                m_pImpl->m_pProps->sPar2,
                nType,
                m_pImpl->m_pProps->nFormat);
           ((SwTblField*)pFld)->ChgExpStr(m_pImpl->m_pProps->sPar1);
        }
        break;
        default: OSL_FAIL("was ist das fuer ein Typ?");
    }
    if (pFld)
    {
        pFld->SetAutomaticLanguage(!m_pImpl->m_pProps->bBool4);
        SwFmtFld aFmt( *pFld );

        UnoActionContext aCont(pDoc);
        SwTxtAttr* pTxtAttr = 0;
        if (aPam.HasMark() &&
            m_pImpl->m_nServiceId != SW_SERVICE_FIELDTYPE_ANNOTATION)
        {
            pDoc->DeleteAndJoin(aPam);
        }

        SwXTextCursor const*const pTextCursor(
                dynamic_cast<SwXTextCursor*>(pCursor));
        const bool bForceExpandHints( (pTextCursor)
                ? pTextCursor->IsAtEndOfMeta() : false );
        const SetAttrMode nInsertFlags = (bForceExpandHints)
            ? nsSetAttrMode::SETATTR_FORCEHINTEXPAND
            : nsSetAttrMode::SETATTR_DEFAULT;

        if (*aPam.GetPoint() != *aPam.GetMark() &&
            m_pImpl->m_nServiceId == SW_SERVICE_FIELDTYPE_ANNOTATION)
        {
            IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
            sw::mark::IFieldmark* pFieldmark = pMarksAccess->makeFieldBookmark(
                    aPam,
                    OUString(),
                    ODF_COMMENTRANGE);
            SwPostItField* pPostItField = (SwPostItField*)aFmt.GetFld();
            if (pPostItField->GetName().isEmpty())
                // The fieldmark always has a (generated) name.
                pPostItField->SetName(pFieldmark->GetName());
            else
                // The field has a name already, use it.
                pMarksAccess->renameMark(pFieldmark, pPostItField->GetName());

            // Make sure we always insert the field at the end
            SwPaM aEnd(*aPam.End(), *aPam.End());
            pDoc->InsertPoolItem(aEnd, aFmt, nInsertFlags);
        }
        else
            pDoc->InsertPoolItem(aPam, aFmt, nInsertFlags);

        pTxtAttr = aPam.GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
                aPam.GetPoint()->nContent.GetIndex()-1, RES_TXTATR_FIELD);

        // was passiert mit dem Update der Felder ? (siehe fldmgr.cxx)
        if (pTxtAttr)
        {
            const SwFmtFld& rFld = pTxtAttr->GetFld();
            m_pImpl->m_pFmtFld = &rFld;
        }
    }
    delete pFld;

    m_pImpl->m_pDoc = pDoc;
    m_pImpl->m_pDoc->GetUnoCallBack()->Add(m_pImpl.get());
    m_pImpl->m_bIsDescriptor = false;
    if (m_pImpl->m_FieldTypeClient.GetRegisteredIn())
    {
        const_cast<SwModify*>(m_pImpl->m_FieldTypeClient.GetRegisteredIn())
            ->Remove(&m_pImpl->m_FieldTypeClient);
    }
    m_pImpl->m_pProps.reset();
    if (m_pImpl->m_bCallUpdate)
        update();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextField::getAnchor() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    SwField const*const pField = m_pImpl->GetField();
    if (!pField)
        return 0;

    const SwTxtFld* pTxtFld = m_pImpl->m_pFmtFld->GetTxtFld();
    if (!pTxtFld)
        throw uno::RuntimeException();
    const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();

    SwPaM aPam(rTxtNode, *pTxtFld->GetStart() + 1, rTxtNode, *pTxtFld->GetStart());

    uno::Reference<text::XTextRange> xRange = SwXTextRange::CreateXTextRange(
            *m_pImpl->m_pDoc, *aPam.GetPoint(), aPam.GetMark());
    return xRange;
}

void SAL_CALL SwXTextField::dispose() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwField const*const pField = m_pImpl->GetField();
    if(pField)
    {
        UnoActionContext aContext(m_pImpl->m_pDoc);
        const SwTxtFld* pTxtFld = m_pImpl->m_pFmtFld->GetTxtFld();
        SwTxtNode& rTxtNode = (SwTxtNode&)*pTxtFld->GetpTxtNode();
        SwPaM aPam(rTxtNode, *pTxtFld->GetStart());
        aPam.SetMark();
        aPam.Move();
        m_pImpl->m_pDoc->DeleteAndJoin(aPam);
    }

    if (m_pImpl->m_pTextObject)
    {
        m_pImpl->m_pTextObject->DisposeEditSource();
        m_pImpl->m_pTextObject->release();
        m_pImpl->m_pTextObject = 0;
    }
}

void SAL_CALL SwXTextField::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXTextField::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextField::getPropertySetInfo()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    //kein static
    uno::Reference< beans::XPropertySetInfo >  aRef;
    if (m_pImpl->m_nServiceId == USHRT_MAX)
    {
        throw uno::RuntimeException();
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(
                    lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    uno::Reference<beans::XPropertySetInfo> xInfo = pPropSet->getPropertySetInfo();
    // extend PropertySetInfo!
    const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
    aRef = new SfxExtItemPropertySetInfo(
        aSwMapProvider.GetPropertyMapEntries(PROPERTY_MAP_PARAGRAPH_EXTENSIONS),
        aPropSeq );
    return aRef;
}

void SAL_CALL
SwXTextField::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
throw (beans::UnknownPropertyException, beans::PropertyVetoException,
    lang::IllegalArgumentException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwField const*const pField = m_pImpl->GetField();
    const SfxItemPropertySet* _pPropSet = aSwMapProvider.GetPropertySet(
                lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    const SfxItemPropertySimpleEntry*   pEntry = _pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
        throw beans::PropertyVetoException ("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pField)
    {
        // special treatment for mail merge fields
        sal_uInt16 nWhich = pField->Which();
        if( RES_DBFLD == nWhich &&
            (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_NAME)) ||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_BASE_URL))||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_TABLE_NAME))||
            rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DATA_COLUMN_NAME))))
        {
            // here a new field type must be created and the field must
            // be registered at the new type
            OSL_FAIL("not implemented");
        }
        else
        {
            SwDoc * pDoc = m_pImpl->m_pDoc;
            assert(pDoc);
            const SwTxtFld* pTxtFld = m_pImpl->m_pFmtFld->GetTxtFld();
            if(!pTxtFld)
                throw uno::RuntimeException();
            SwPosition aPosition( pTxtFld->GetTxtNode() );
            aPosition.nContent = *pTxtFld->GetStart();
            pDoc->PutValueToField( aPosition, rValue, pEntry->nWID);
        }

        //#i100374# notify SwPostIt about new field content
        if (RES_POSTITFLD == nWhich && m_pImpl->m_pFmtFld)
        {
            const_cast<SwFmtFld*>(m_pImpl->m_pFmtFld)->Broadcast(
                    SwFmtFldHint( 0, SWFMTFLD_CHANGED ));
        }

        // fdo#42073 notify SwTxtFld about changes of the expanded string
        if (m_pImpl->m_pFmtFld->GetTxtFld())
        {
            m_pImpl->m_pFmtFld->GetTxtFld()->Expand();
        }

        //#i100374# changing a document field should set the modify flag
        SwDoc* pDoc = m_pImpl->m_pDoc;
        if (pDoc)
            pDoc->SetModified();

    }
    else if (m_pImpl->m_pProps)
    {
        sal_Bool* pBool = 0;
        switch(pEntry->nWID)
        {
        case FIELD_PROP_PAR1:
            rValue >>= m_pImpl->m_pProps->sPar1;
            break;
        case FIELD_PROP_PAR2:
            rValue >>= m_pImpl->m_pProps->sPar2;
            break;
        case FIELD_PROP_PAR3:
            rValue >>= m_pImpl->m_pProps->sPar3;
            break;
        case FIELD_PROP_PAR4:
            rValue >>= m_pImpl->m_pProps->sPar4;
            break;
        case FIELD_PROP_FORMAT:
            rValue >>= m_pImpl->m_pProps->nFormat;
            m_pImpl->m_pProps->bFormatIsDefault = false;
            break;
        case FIELD_PROP_SUBTYPE:
            m_pImpl->m_pProps->nSubType = SWUnoHelper::GetEnumAsInt32(rValue);
            break;
        case FIELD_PROP_BYTE1 :
            rValue >>= m_pImpl->m_pProps->nByte1;
            break;
        case FIELD_PROP_BOOL1 :
            pBool = &m_pImpl->m_pProps->bBool1;
            break;
        case FIELD_PROP_BOOL2 :
            pBool = &m_pImpl->m_pProps->bBool2;
            break;
        case FIELD_PROP_BOOL3 :
            pBool = &m_pImpl->m_pProps->bBool3;
            break;
        case FIELD_PROP_BOOL4:
            pBool = &m_pImpl->m_pProps->bBool4;
        break;
        case FIELD_PROP_DATE :
        {
            if(rValue.getValueType() != ::getCppuType(static_cast<const util::Date*>(0)))
                throw lang::IllegalArgumentException();

            util::Date aTemp = *(const util::Date*)rValue.getValue();
            m_pImpl->m_pProps->aDate = Date(aTemp.Day, aTemp.Month, aTemp.Year);
        }
        break;
        case FIELD_PROP_USHORT1:
        case FIELD_PROP_USHORT2:
            {
                 sal_Int16 nVal = 0;
                rValue >>= nVal;
                if( FIELD_PROP_USHORT1 == pEntry->nWID)
                    m_pImpl->m_pProps->nUSHORT1 = nVal;
                else
                    m_pImpl->m_pProps->nUSHORT2 = nVal;
            }
            break;
        case FIELD_PROP_SHORT1:
            rValue >>= m_pImpl->m_pProps->nSHORT1;
            break;
        case FIELD_PROP_DOUBLE:
            if(rValue.getValueType() != ::getCppuType(static_cast<const double*>(0)))
                throw lang::IllegalArgumentException();
            rValue >>= m_pImpl->m_pProps->fDouble;
            break;

        case FIELD_PROP_DATE_TIME :
            if (!m_pImpl->m_pProps->pDateTime)
                m_pImpl->m_pProps->pDateTime = new util::DateTime;
            rValue >>= (*m_pImpl->m_pProps->pDateTime);
            break;
        case FIELD_PROP_PROP_SEQ:
            rValue >>= m_pImpl->m_pProps->aPropSeq;
            break;
        case FIELD_PROP_STRINGS:
            rValue >>= m_pImpl->m_pProps->aStrings;
            break;
        }
        if (pBool)
        {
            if( rValue.getValueType() == getCppuBooleanType() )
                *pBool = *(sal_Bool*)rValue.getValue();
            else
                throw lang::IllegalArgumentException();
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL SwXTextField::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwField const*const pField = m_pImpl->GetField();
    const SfxItemPropertySet* _pPropSet = aSwMapProvider.GetPropertySet(
                lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    const SfxItemPropertySimpleEntry*   pEntry = _pPropSet->getPropertyMap().getByName(rPropertyName);
    if(!pEntry )
    {
        const SfxItemPropertySet* _pParaPropSet = aSwMapProvider.GetPropertySet(PROPERTY_MAP_PARAGRAPH_EXTENSIONS);
        pEntry = _pParaPropSet->getPropertyMap().getByName(rPropertyName);
    }
    if (!pEntry)
        throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    switch( pEntry->nWID )
    {
    case FN_UNO_TEXT_WRAP:
        aRet <<= text::WrapTextMode_NONE;
        break;
    case FN_UNO_ANCHOR_TYPE:
        aRet <<= text::TextContentAnchorType_AS_CHARACTER;
        break;
    case FN_UNO_ANCHOR_TYPES:
        {
            uno::Sequence<text::TextContentAnchorType> aTypes(1);
            text::TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = text::TextContentAnchorType_AS_CHARACTER;
            aRet.setValue(&aTypes, ::getCppuType(static_cast<uno::Sequence<text::TextContentAnchorType>*>(0)));
        }
        break;

    default:
        if( pField )
        {
            if (FIELD_PROP_IS_FIELD_USED      == pEntry->nWID ||
                FIELD_PROP_IS_FIELD_DISPLAYED == pEntry->nWID)
            {
                sal_Bool bIsFieldUsed       = sal_False;
                sal_Bool bIsFieldDisplayed  = sal_False;

                // in order to have the information about fields
                // correctly evaluated the document needs a layout
                // (has to be already formatted)
                SwDoc *pDoc = m_pImpl->m_pDoc;
                ViewShell *pViewShell = 0;
                SwEditShell *pEditShell = pDoc ? pDoc->GetEditShell( &pViewShell ) : 0;
                if (pEditShell)
                    pEditShell->CalcLayout();
                else if (pViewShell) // a page preview has no SwEditShell it should only have a view shell
                    pViewShell->CalcLayout();
                else
                    throw uno::RuntimeException();

                // get text node for the text field
                const SwFmtFld *pFldFmt =
                    (m_pImpl->GetField()) ? m_pImpl->m_pFmtFld : 0;
                const SwTxtFld* pTxtFld = (pFldFmt)
                    ? m_pImpl->m_pFmtFld->GetTxtFld() : 0;
                if(!pTxtFld)
                    throw uno::RuntimeException();
                const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();

                // skip fields that are currently not in the document
                // e.g. fields in undo or redo array
                if (rTxtNode.GetNodes().IsDocNodes())
                {
                    sal_Bool bFrame = 0 != rTxtNode.FindLayoutRect().Width(); // oder so
                    sal_Bool bHidden = rTxtNode.IsHidden();
                    if ( !bHidden )
                    {
                        xub_StrLen nHiddenStart;
                        xub_StrLen nHiddenEnd;

                        SwPosition aPosition( pTxtFld->GetTxtNode() );
                        aPosition.nContent = *pTxtFld->GetStart();

                        bHidden = SwScriptInfo::GetBoundsOfHiddenRange( pTxtFld->GetTxtNode(),
                                        *pTxtFld->GetStart(),
                                        nHiddenStart, nHiddenEnd );
                    }

                    // !bFrame && !bHidden: aller Wahrscheinlichkeit handelt es
                    // sich um ein Feld in einem unbenutzten Seitenstyle
                    //
                    // bHidden: Feld ist versteckt
                    // FME: Problem: Verstecktes Feld in unbenutzter Seitenvorlage =>
                    // bIsFieldUsed = true
                    // bIsFieldDisplayed = false
                    bIsFieldUsed       = bFrame || bHidden;
                    bIsFieldDisplayed  = bIsFieldUsed && !bHidden;
                }
                sal_Bool bRetVal = (FIELD_PROP_IS_FIELD_USED == pEntry->nWID) ?
                                            bIsFieldUsed : bIsFieldDisplayed;
                aRet.setValue( &bRetVal, ::getCppuBooleanType() );
            }
            else
                pField->QueryValue( aRet, pEntry->nWID );
        }
        else if (m_pImpl->m_pProps)     // currently just a descriptor...
        {
            switch(pEntry->nWID)
            {
            case FIELD_PROP_TEXT:
                {
                    if (!m_pImpl->m_pTextObject)
                    {
                        SwTextAPIEditSource* pObj =
                            new SwTextAPIEditSource(m_pImpl->m_pDoc);
                        m_pImpl->m_pTextObject = new SwTextAPIObject( pObj );
                        m_pImpl->m_pTextObject->acquire();
                    }

                    uno::Reference<text::XText> xText(m_pImpl->m_pTextObject);
                    aRet <<= xText;
                    break;
                }
            case FIELD_PROP_PAR1:
                aRet <<= m_pImpl->m_pProps->sPar1;
                break;
            case FIELD_PROP_PAR2:
                aRet <<= m_pImpl->m_pProps->sPar2;
                break;
            case FIELD_PROP_PAR3:
                aRet <<= m_pImpl->m_pProps->sPar3;
                break;
            case FIELD_PROP_PAR4:
                aRet <<= m_pImpl->m_pProps->sPar4;
                break;
            case FIELD_PROP_FORMAT:
                aRet <<= m_pImpl->m_pProps->nFormat;
                break;
            case FIELD_PROP_SUBTYPE:
                aRet <<= m_pImpl->m_pProps->nSubType;
                break;
            case FIELD_PROP_BYTE1 :
                aRet <<= m_pImpl->m_pProps->nByte1;
                break;
            case FIELD_PROP_BOOL1 :
                aRet.setValue(&m_pImpl->m_pProps->bBool1, ::getCppuBooleanType());
                break;
            case FIELD_PROP_BOOL2 :
                aRet.setValue(&m_pImpl->m_pProps->bBool2, ::getCppuBooleanType());
                break;
            case FIELD_PROP_BOOL3 :
                aRet.setValue(&m_pImpl->m_pProps->bBool3, ::getCppuBooleanType());
                break;
            case FIELD_PROP_BOOL4 :
                aRet.setValue(&m_pImpl->m_pProps->bBool4, ::getCppuBooleanType());
                break;
            case FIELD_PROP_DATE :
                aRet.setValue(&m_pImpl->m_pProps->aDate, ::getCppuType(static_cast<const util::Date*>(0)));
                break;
            case FIELD_PROP_USHORT1:
                aRet <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
                break;
            case FIELD_PROP_USHORT2:
                aRet <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT2);
                break;
            case FIELD_PROP_SHORT1:
                aRet <<= m_pImpl->m_pProps->nSHORT1;
                break;
            case FIELD_PROP_DOUBLE:
                aRet <<= m_pImpl->m_pProps->fDouble;
                break;
            case FIELD_PROP_DATE_TIME :
                if (m_pImpl->m_pProps->pDateTime)
                    aRet <<= (*m_pImpl->m_pProps->pDateTime);
                break;
            case FIELD_PROP_PROP_SEQ:
                aRet <<= m_pImpl->m_pProps->aPropSeq;
                break;
            case FIELD_PROP_STRINGS:
                aRet <<= m_pImpl->m_pProps->aStrings;
                break;
            case FIELD_PROP_IS_FIELD_USED:
            case FIELD_PROP_IS_FIELD_DISPLAYED:
                aRet <<= false;
                break;
            }
        }
        else
            throw uno::RuntimeException();
    }
    return aRet;
}

void SwXTextField::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextField::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextField::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextField::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextField::update() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwField const*const pFld = m_pImpl->GetField();
    if (pFld)
    {
        switch(pFld->Which())
        {
            case RES_DATETIMEFLD:
            ((SwDateTimeField*)pFld)->SetDateTime( ::DateTime( ::DateTime::SYSTEM ) );
            break;

            case RES_EXTUSERFLD:
            {
                SwExtUserField* pExtUserFld = (SwExtUserField*)pFld;
                pExtUserFld->SetExpansion( ((SwExtUserFieldType*)pFld->GetTyp())->Expand(
                                            pExtUserFld->GetSubType(),
                                            pExtUserFld->GetFormat() ) );
            }
            break;

            case RES_AUTHORFLD:
            {
                SwAuthorField* pAuthorFld = (SwAuthorField*)pFld;
                pAuthorFld->SetExpansion( ((SwAuthorFieldType*)pFld->GetTyp())->Expand(
                                            pAuthorFld->GetFormat() ) );
            }
            break;

            case RES_FILENAMEFLD:
            {
                SwFileNameField* pFileNameFld = (SwFileNameField*)pFld;
                pFileNameFld->SetExpansion( ((SwFileNameFieldType*)pFld->GetTyp())->Expand(
                                            pFileNameFld->GetFormat() ) );
            }
            break;

            case RES_DOCINFOFLD:
            {
                    SwDocInfoField* pDocInfFld = (SwDocInfoField*)pFld;
                    pDocInfFld->SetExpansion( ((SwDocInfoFieldType*)pFld->GetTyp())->Expand(
                                                pDocInfFld->GetSubType(),
                                                pDocInfFld->GetFormat(),
                                                pDocInfFld->GetLanguage(),
                                                pDocInfFld->GetName() ) );
            }
            break;
        }
        // Text formatting has to be triggered.
        const_cast<SwFmtFld*>(m_pImpl->m_pFmtFld)->ModifyNotification(0, 0);
    }
    else
        m_pImpl->m_bCallUpdate = true;
}

OUString SAL_CALL SwXTextField::getImplementationName()
throw (uno::RuntimeException)
{
    return OUString("SwXTextField");
}

static OUString OldNameToNewName_Impl( const OUString &rOld )
{
    static OUString aOldNamePart1(".TextField.DocInfo.");
    static OUString aOldNamePart2(".TextField.");
    static OUString aNewNamePart1(".textfield.docinfo.");
    static OUString aNewNamePart2(".textfield.");
    OUString sServiceNameCC( rOld );
    sal_Int32 nIdx = sServiceNameCC.indexOf( aOldNamePart1 );
    if (nIdx >= 0)
        sServiceNameCC = sServiceNameCC.replaceAt( nIdx, aOldNamePart1.getLength(), aNewNamePart1 );
    nIdx = sServiceNameCC.indexOf( aOldNamePart2 );
    if (nIdx >= 0)
        sServiceNameCC = sServiceNameCC.replaceAt( nIdx, aOldNamePart2.getLength(), aNewNamePart2 );
    return sServiceNameCC;
}

sal_Bool SAL_CALL SwXTextField::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    OUString sServiceName =
        SwXServiceProvider::GetProviderName(m_pImpl->m_nServiceId);

    // case-corected version of service-name (see #i67811)
    // (need to supply both because of compatibility to older versions)
    OUString sServiceNameCC(  OldNameToNewName_Impl( sServiceName ) );

    return sServiceName == rServiceName || sServiceNameCC == rServiceName ||
        rServiceName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextContent"));
}

uno::Sequence< OUString > SAL_CALL SwXTextField::getSupportedServiceNames()
throw (uno::RuntimeException)
{
    OUString sServiceName =
        SwXServiceProvider::GetProviderName(m_pImpl->m_nServiceId);

    // case-corected version of service-name (see #i67811)
    // (need to supply both because of compatibility to older versions)
    OUString sServiceNameCC(  OldNameToNewName_Impl( sServiceName ) );
    sal_Int32 nLen = sServiceName == sServiceNameCC ? 2 : 3;

    uno::Sequence< OUString > aRet( nLen );
    OUString* pArray = aRet.getArray();
    *pArray++ = sServiceName;
    if (nLen == 3)
        *pArray++ = sServiceNameCC;
    *pArray++ = "com.sun.star.text.TextContent";
    return aRet;
}

void SwXTextField::Impl::Invalidate()
{
    if (GetRegisteredIn())
    {
        GetRegisteredInNonConst()->Remove(this);
        m_pFmtFld = 0;
        m_pDoc = 0;
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(m_rThis));
        m_EventListeners.disposeAndClear(ev);
    }
}

void SwXTextField::Impl::Modify(
        SfxPoolItem const*const pOld, SfxPoolItem const*const pNew)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            Invalidate();
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pNew)->pChangedFmt == GetRegisteredIn() &&
            ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
            Invalidate();
        break;
    case RES_FIELD_DELETED:
        if ((void*)m_pFmtFld == ((SwPtrMsgPoolItem *)pOld)->pObject)
            Invalidate();
        break;
    }
}

const SwField*  SwXTextField::Impl::GetField() const
{
    if (GetRegisteredIn() && m_pFmtFld)
        return m_pFmtFld->GetFld();
    return 0;
}

/******************************************************************
 * SwXTextFieldMasters
 ******************************************************************/
OUString SwXTextFieldMasters::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextFieldMasters");
}

sal_Bool SwXTextFieldMasters::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == "com.sun.star.text.TextFieldMasters";
}

uno::Sequence< OUString > SwXTextFieldMasters::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextFieldMasters";
    return aRet;
}

SwXTextFieldMasters::SwXTextFieldMasters(SwDoc* _pDoc) :
    SwUnoCollection(_pDoc)
{
}

SwXTextFieldMasters::~SwXTextFieldMasters()
{

}

/*-----------------------------------------------------------------------
    Iteration ueber nicht-Standard Feldtypen
    USER/SETEXP/DDE/DATABASE
    Der Name ist demnach:
    "com.sun.star.text.fieldmaster.User" + <Feltypname>
    "com.sun.star.text.fieldmaster.DDE" + <Feltypname>
    "com.sun.star.text.fieldmaster.SetExpression" + <Feltypname>
    "com.sun.star.text.fieldmaster.DataBase" + <Feltypname>

    Falls wir grosszuegig werden wollen, dann koennte man com.sun.star.text
    auch optional weglassen
  -----------------------------------------------------------------------*/
static sal_uInt16 lcl_GetIdByName( String& rName, String& rTypeName )
{
    if( rName.EqualsAscii( COM_TEXT_FLDMASTER, 0, RTL_CONSTASCII_LENGTH(COM_TEXT_FLDMASTER ))
        ||  rName.EqualsAscii( COM_TEXT_FLDMASTER_CC, 0, RTL_CONSTASCII_LENGTH(COM_TEXT_FLDMASTER_CC )))
        rName.Erase(0, 30);

    sal_uInt16 nResId = USHRT_MAX;
    sal_Int32 nFound = 0;
    rTypeName = rName.GetToken( 0, '.', nFound );
    if(rTypeName.EqualsAscii("User"))
        nResId = RES_USERFLD;
    else if(rTypeName.EqualsAscii("DDE"))
        nResId = RES_DDEFLD;
    else if(rTypeName.EqualsAscii("SetExpression"))
    {
        nResId = RES_SETEXPFLD;

        String sFldTypName( rName.GetToken( 1, '.' ));
        String sUIName( SwStyleNameMapper::GetSpecialExtraUIName( sFldTypName ) );

        if( sUIName != sFldTypName )
            rName.SetToken( 1, '.', sUIName );
    }
    else if(rTypeName.EqualsAscii("DataBase"))
    {
        rName.Erase( 0, RTL_CONSTASCII_LENGTH( "DataBase." ));
        sal_uInt16 nDotCount = comphelper::string::getTokenCount(rName, '.');
        if( 2 <= nDotCount )
        {
            // #i51815#
            //rName.SearchAndReplace('.', DB_DELIM);
            //rName.SetChar( rName.SearchBackward( '.' ), DB_DELIM );

            rName.InsertAscii( "DataBase.", 0 );
            nResId = RES_DBFLD;
        }
    }
    else if( rTypeName.EqualsAscii("Bibliography"))
        nResId = RES_AUTHORITY;
    return nResId;
}

uno::Any SwXTextFieldMasters::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    String sName(rName), sTypeName;
    sal_uInt16 nResId = lcl_GetIdByName( sName, sTypeName );
    if( USHRT_MAX == nResId )
        throw container::NoSuchElementException();

    sName.Erase(0, sTypeName.Len()+1);
    SwFieldType* pType = GetDoc()->GetFldType(nResId, sName, sal_True);
    if(!pType)
        throw container::NoSuchElementException();

    uno::Reference<beans::XPropertySet> const xRet(
            SwXFieldMaster::CreateXFieldMaster(*GetDoc(), *pType));
    return uno::makeAny(xRet);
}

sal_Bool SwXTextFieldMasters::getInstanceName(
    const SwFieldType& rFldType, OUString& rName)
{
    OUString sField;

    switch( rFldType.Which() )
    {
    case RES_USERFLD:
        sField = "User." + rFldType.GetName();
        break;
    case RES_DDEFLD:
        sField = "DDE." + rFldType.GetName();
        break;

    case RES_SETEXPFLD:
        sField = "SetExpression." + SwStyleNameMapper::GetSpecialExtraProgName( rFldType.GetName() );
        break;

    case RES_DBFLD:
        sField = "DataBase." + rFldType.GetName().replaceAll(OUString(DB_DELIM), ".");
        break;

    case RES_AUTHORITY:
        sField = "Bibliography";
        break;

    default:
        return sal_False;
    }

    rName += COM_TEXT_FLDMASTER_CC + sField;
    return sal_True;
}

uno::Sequence< OUString > SwXTextFieldMasters::getElementNames(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    sal_uInt16 nCount = pFldTypes->size();

    std::vector<OUString*> aFldNames;
    OUString* pString = new OUString();

    for( sal_uInt16 i = 0; i < nCount; i++)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);

        if (SwXTextFieldMasters::getInstanceName(rFldType, *pString))
        {
            aFldNames.push_back(pString);
            pString = new OUString();
        }
    }
    delete pString;

    uno::Sequence< OUString > aSeq( static_cast<sal_uInt16>(aFldNames.size()) );
    OUString* pArray = aSeq.getArray();
    for(sal_uInt16 i = 0; i < aFldNames.size();i++)
    {
        pArray[i] = *aFldNames[i];
        delete aFldNames[i];
    }

    return aSeq;
}

sal_Bool SwXTextFieldMasters::hasByName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    String sName(rName), sTypeName;
    sal_uInt16 nResId = lcl_GetIdByName( sName, sTypeName );
    sal_Bool bRet = sal_False;
    if( USHRT_MAX != nResId )
    {
        sName.Erase(0, sTypeName.Len()+1);
        bRet = USHRT_MAX != nResId && 0 != GetDoc()->GetFldType(nResId, sName, sal_True);
    }
    return bRet;
}

uno::Type  SwXTextFieldMasters::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType(static_cast<const uno::Reference<beans::XPropertySet>*>(0));

}

sal_Bool SwXTextFieldMasters::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return sal_True;
}

/******************************************************************
 * SwXFieldTypes
 ******************************************************************/

class SwXTextFieldTypes::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    ::cppu::OInterfaceContainerHelper m_RefreshListeners;

    Impl() : m_RefreshListeners(m_Mutex) { }
};

OUString SwXTextFieldTypes::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextFieldTypes");
}

sal_Bool SwXTextFieldTypes::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == "com.sun.star.text.TextFields";
}

uno::Sequence< OUString > SwXTextFieldTypes::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextFields";
    return aRet;
}

SwXTextFieldTypes::SwXTextFieldTypes(SwDoc* _pDoc)
    : SwUnoCollection (_pDoc)
    , m_pImpl(new Impl)
{
}

SwXTextFieldTypes::~SwXTextFieldTypes()
{
}

void SwXTextFieldTypes::Invalidate()
{
    SwUnoCollection::Invalidate();
    lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
    m_pImpl->m_RefreshListeners.disposeAndClear(ev);
}

uno::Reference< container::XEnumeration >  SwXTextFieldTypes::createEnumeration(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return new SwXFieldEnumeration(*GetDoc());
}

uno::Type  SwXTextFieldTypes::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType(static_cast<const uno::Reference<text::XDependentTextField>*>(0));
}

sal_Bool SwXTextFieldTypes::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return sal_True; // they always exist
}

void SAL_CALL SwXTextFieldTypes::refresh() throw (uno::RuntimeException)
{
    {
        SolarMutexGuard aGuard;
        if (!IsValid())
            throw uno::RuntimeException();
        UnoActionContext aContext(GetDoc());
        GetDoc()->UpdateDocStat();
        GetDoc()->UpdateFlds(0, sal_False);
    }
    // call refresh listeners (without SolarMutex locked)
    lang::EventObject const event(static_cast< ::cppu::OWeakObject*>(this));
    m_pImpl->m_RefreshListeners.notifyEach(
            & util::XRefreshListener::refreshed, event);
}

void SAL_CALL SwXTextFieldTypes::addRefreshListener(
        const uno::Reference<util::XRefreshListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.addInterface(xListener);
}

void SAL_CALL SwXTextFieldTypes::removeRefreshListener(
        const uno::Reference<util::XRefreshListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.removeInterface(xListener);
}

/******************************************************************
 * SwXFieldEnumeration
 ******************************************************************/

class SwXFieldEnumeration::Impl
    : public SwClient
{

public:
    SwDoc * m_pDoc;

    uno::Sequence< uno::Reference<text::XTextField> > m_Items;
    sal_Int32       m_nNextIndex;  ///< index of next element to be returned

    Impl(SwDoc & rDoc)
        : SwClient(rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD))
        , m_pDoc(& rDoc)
        , m_nNextIndex(0)
    { }

protected:
    // SwClient
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew);
};

OUString SAL_CALL
SwXFieldEnumeration::getImplementationName() throw (uno::RuntimeException)
{
    return OUString("SwXFieldEnumeration");
}

sal_Bool SAL_CALL
SwXFieldEnumeration::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException)
{
    return rServiceName == "com.sun.star.text.FieldEnumeration";
}

uno::Sequence< OUString > SAL_CALL
SwXFieldEnumeration::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.FieldEnumeration";
    return aRet;
}

SwXFieldEnumeration::SwXFieldEnumeration(SwDoc & rDoc)
    : m_pImpl(new Impl(rDoc))
{
    // build sequence
    sal_Int32 nSize = 32;
    m_pImpl->m_Items.realloc( nSize );
    uno::Reference< text::XTextField > *pItems = m_pImpl->m_Items.getArray();
    sal_Int32 nFillPos = 0;
    //
    const SwFldTypes* pFldTypes = m_pImpl->m_pDoc->GetFldTypes();
    sal_uInt16 nCount = pFldTypes->size();
    for(sal_uInt16 nType = 0;  nType < nCount;  ++nType)
    {
        const SwFieldType *pCurType = (*pFldTypes)[nType];
        SwIterator<SwFmtFld,SwFieldType> aIter( *pCurType );
        const SwFmtFld* pCurFldFmt = aIter.First();
        while (pCurFldFmt)
        {
            const SwTxtFld *pTxtFld = pCurFldFmt->GetTxtFld();
            // skip fields that are currently not in the document
            // e.g. fields in undo or redo array
            bool bSkip = !pTxtFld ||
                         !pTxtFld->GetpTxtNode()->GetNodes().IsDocNodes();
            if (!bSkip)
                pItems[ nFillPos++ ] = SwXTextField::CreateXTextField(
                        *m_pImpl->m_pDoc, *pCurFldFmt);
            pCurFldFmt = aIter.Next();

            // enlarge sequence if necessary
            if (m_pImpl->m_Items.getLength() == nFillPos)
            {
                m_pImpl->m_Items.realloc( 2 * m_pImpl->m_Items.getLength() );
                pItems = m_pImpl->m_Items.getArray();
            }
        }
    }
    // now handle meta-fields, which are not SwFields
    const ::std::vector< uno::Reference<text::XTextField> > MetaFields(
           m_pImpl->m_pDoc->GetMetaFieldManager().getMetaFields() );
    for (size_t i = 0; i < MetaFields.size(); ++i)
    {
        pItems[ nFillPos ] = MetaFields[i];
        nFillPos++;

        //FIXME UGLY
        // enlarge sequence if necessary
        if (m_pImpl->m_Items.getLength() == nFillPos)
        {
            m_pImpl->m_Items.realloc( 2 * m_pImpl->m_Items.getLength() );
            pItems = m_pImpl->m_Items.getArray();
        }
    }
    // resize sequence to actual used size
    m_pImpl->m_Items.realloc( nFillPos );
}

SwXFieldEnumeration::~SwXFieldEnumeration()
{
}

sal_Bool SAL_CALL SwXFieldEnumeration::hasMoreElements()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return m_pImpl->m_nNextIndex < m_pImpl->m_Items.getLength();
}

uno::Any SAL_CALL SwXFieldEnumeration::nextElement()
throw (container::NoSuchElementException, lang::WrappedTargetException,
        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (!(m_pImpl->m_nNextIndex < m_pImpl->m_Items.getLength()))
        throw container::NoSuchElementException();

#if OSL_DEBUG_LEVEL > 1
    uno::Reference< text::XTextField > *pItems = m_pImpl->m_Items.getArray();
    (void)pItems;
#endif
    uno::Reference< text::XTextField >  &rxFld =
        m_pImpl->m_Items.getArray()[ m_pImpl->m_nNextIndex++ ];
    uno::Any aRet;
    aRet <<= rxFld;
    rxFld = 0;  // free memory for item that is not longer used
    return aRet;
}

void SwXFieldEnumeration::Impl::Modify(
        SfxPoolItem const*const pOld, SfxPoolItem const*const pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        m_pDoc = 0;
}

String& GetString( const uno::Any& rAny, String& rStr )
{
    OUString aStr;
    rAny >>= aStr;
    rStr = String( aStr );
    return rStr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
