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

#include <config_features.h>

#include <sal/config.h>

#include <algorithm>
#include <memory>

#include <unofield.hxx>
#include <unofieldcoll.hxx>
#include <swtypes.hxx>
#include <cmdid.h>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
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
#include <cppuhelper/supportsservice.hxx>
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
#include <fmtmeta.hxx>
#include <calbck.hxx>
#include <rtl/strbuf.hxx>
#include <vector>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

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

static sal_uInt16 lcl_GetServiceForField( const SwField& rField )
{
    const sal_uInt16 nWhich = rField.Which();
    sal_uInt16 nSrvId = USHRT_MAX;
    //special handling for some fields
    switch( nWhich )
    {
    case RES_INPUTFLD:
        if( INP_USR == (rField.GetSubType() & 0x00ff) )
            nSrvId = SW_SERVICE_FIELDTYPE_INPUT_USER;
        break;

    case RES_DOCINFOFLD:
        {
            const sal_uInt16 nSubType = rField.GetSubType();
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
        nSrvId = TYP_CONDTXTFLD == rField.GetSubType()
                        ? SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT
                        : SW_SERVICE_FIELDTYPE_HIDDEN_TEXT;
        break;

    case RES_DOCSTATFLD:
        {
            switch( rField.GetSubType() )
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

static sal_Int32 lcl_PropName2TokenPos(const OUString& rPropertyName)
{
    if (rPropertyName == UNO_NAME_DDE_COMMAND_TYPE)
        return 0;

    if (rPropertyName == UNO_NAME_DDE_COMMAND_FILE)
        return 1;

    if (rPropertyName == UNO_NAME_DDE_COMMAND_ELEMENT)
        return 2;

    if (rPropertyName == UNO_NAME_IS_AUTOMATIC_UPDATE)
        return 3;

    return SAL_MAX_INT32;
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

class SwXFieldMaster::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    uno::WeakReference<uno::XInterface> m_wThis;
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
    bool        m_bParam1;  // IsExpression
    sal_Int32       m_nParam2;

    Impl(SwModify *const pModify,
            SwDoc * pDoc, sal_uInt16 const nResId, bool const bIsDescriptor)
        : SwClient(pModify)
        , m_EventListeners(m_Mutex)
        , m_pDoc(pDoc)
        , m_bIsDescriptor(bIsDescriptor)
        , m_nResTypeId(nResId)
        , m_fParam1(0.0)
        , m_nParam1(-1)
        , m_bParam1(false)
        , m_nParam2(0)
    { }

protected:
    // SwClient
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew) override;
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
throw (uno::RuntimeException, std::exception)
{
    return ::sw::UnoTunnelImpl<SwXFieldMaster>(rId, this);
}

OUString SAL_CALL
SwXFieldMaster::getImplementationName()
throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXFieldMaster");
}

namespace
{

OUString getServiceName(const sal_uInt16 aId)
{
    const sal_Char* pEntry;
    switch (aId)
    {
        case RES_USERFLD:
            pEntry = "User";
            break;
        case RES_DBFLD:
            pEntry = "Database";
            break;
        case RES_SETEXPFLD:
            pEntry = "SetExpression";
            break;
        case RES_DDEFLD:
            pEntry = "DDE";
            break;
        case RES_AUTHORITY:
            pEntry = "Bibliography";
            break;
        default:
            return OUString();
    }

    return "com.sun.star.text.fieldmaster." + OUString::createFromAscii(pEntry);
}

}

sal_Bool SAL_CALL SwXFieldMaster::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXFieldMaster::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextFieldMaster";
    pArray[1] = getServiceName(m_pImpl->m_nResTypeId);
    return aRet;
}

SwXFieldMaster::SwXFieldMaster(SwDoc *const pDoc, sal_uInt16 const nResId)
    : m_pImpl(new Impl(pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD),
                pDoc, nResId, true))
{
}

SwXFieldMaster::SwXFieldMaster(SwFieldType& rType, SwDoc * pDoc)
    : m_pImpl(new Impl(&rType, pDoc, rType.Which(), false))
{
}

SwXFieldMaster::~SwXFieldMaster()
{
}

uno::Reference<beans::XPropertySet>
SwXFieldMaster::CreateXFieldMaster(SwDoc * pDoc, SwFieldType *const pType,
        sal_uInt16 nResId)
{
    // re-use existing SwXFieldMaster
    uno::Reference<beans::XPropertySet> xFM;
    if (pType)
    {
        xFM = pType->GetXObject();
    }
    if (!xFM.is())
    {
        SwXFieldMaster *const pFM( (pType)
                ? new SwXFieldMaster(*pType, pDoc)
                : new SwXFieldMaster(pDoc, nResId));
        xFM.set(pFM);
        if (pType)
        {
            pType->SetXObject(xFM);
        }
        // need a permanent Reference to initialize m_wThis
        pFM->m_pImpl->m_wThis = xFM;
    }
    return xFM;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL
SwXFieldMaster::getPropertySetInfo()
throw (uno::RuntimeException, std::exception)
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
       uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFieldType* pType = GetFieldType(true);
    if(pType)
    {
        bool bSetValue = true;
        if( rPropertyName == UNO_NAME_SUB_TYPE )
        {
            const ::std::vector<OUString>& rExtraArr(
                    SwStyleNameMapper::GetExtraUINameArray());
            const OUString sTypeName = pType->GetName();
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
        if ( bSetValue )
        {
            // nothing special to be done here for the properties
            // UNO_NAME_DATA_BASE_NAME and UNO_NAME_DATA_BASE_URL.
            // We just call PutValue (empty string is allowed).
            // Thus the last property set will be used as Data Source.

            const sal_uInt16 nMemberValueId = GetFieldTypeMId( rPropertyName, *pType );
            if ( USHRT_MAX != nMemberValueId )
            {
                pType->PutValue( rValue, nMemberValueId );
                if ( pType->Which() == RES_USERFLD )
                {
                    // trigger update of User field in order to get depending Input Fields updated.
                    pType->UpdateFields();
                }
            }
            else
            {
                throw beans::UnknownPropertyException(
                    "Unknown property: " + rPropertyName,
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
    }
    else if (!pType && m_pImpl->m_pDoc && rPropertyName == UNO_NAME_NAME)
    {
        OUString sTypeName;
        rValue >>= sTypeName;
        SwFieldType * pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().GetFieldType(
                m_pImpl->m_nResTypeId, sTypeName, false);

        if(pType2 ||
            (RES_SETEXPFLD == m_pImpl->m_nResTypeId &&
            ( sTypeName == SW_RESSTR(STR_POOLCOLL_LABEL_TABLE) ||
              sTypeName == SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING) ||
              sTypeName == SW_RESSTR(STR_POOLCOLL_LABEL_FRAME) ||
              sTypeName == SW_RESSTR(STR_POOLCOLL_LABEL_ABB) )))
        {
            throw lang::IllegalArgumentException();
        }

        switch (m_pImpl->m_nResTypeId)
        {
            case RES_USERFLD :
            {
                SwUserFieldType aType(m_pImpl->m_pDoc, sTypeName);
                pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
                static_cast<SwUserFieldType*>(pType2)->SetContent(m_pImpl->m_sParam1);
                static_cast<SwUserFieldType*>(pType2)->SetValue(m_pImpl->m_fParam1);
                static_cast<SwUserFieldType*>(pType2)->SetType(m_pImpl->m_bParam1
                    ? nsSwGetSetExpType::GSE_EXPR : nsSwGetSetExpType::GSE_STRING);
            }
            break;
            case RES_DDEFLD :
            {
                SwDDEFieldType aType(sTypeName, m_pImpl->m_sParam1,
                    m_pImpl->m_bParam1 ? SfxLinkUpdateMode::ALWAYS : SfxLinkUpdateMode::ONCALL);
                pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
            }
            break;
            case RES_SETEXPFLD :
            {
                SwSetExpFieldType aType(m_pImpl->m_pDoc, sTypeName);
                if (!m_pImpl->m_sParam1.isEmpty())
                    aType.SetDelimiter(OUString(m_pImpl->m_sParam1[0]));
                if (m_pImpl->m_nParam1 > -1 && m_pImpl->m_nParam1 < MAXLEVEL)
                    aType.SetOutlineLvl(m_pImpl->m_nParam1);
                pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
            }
            break;
            case RES_DBFLD :
            {
                rValue >>= m_pImpl->m_sParam3;
                pType2 = GetFieldType();
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
            if(rPropertyName == UNO_NAME_CONTENT)
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName == UNO_NAME_VALUE)
            {
                if(rValue.getValueType() != ::cppu::UnoType<double>::get())
                    throw lang::IllegalArgumentException();
                rValue >>= m_pImpl->m_fParam1;
            }
            else if(rPropertyName == UNO_NAME_IS_EXPRESSION)
            {
                if(rValue.getValueType() != cppu::UnoType<bool>::get())
                    throw lang::IllegalArgumentException();
                rValue >>= m_pImpl->m_bParam1;
            }

            break;
        case RES_DBFLD:
            if(rPropertyName == UNO_NAME_DATA_BASE_NAME)
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName == UNO_NAME_DATA_TABLE_NAME)
                rValue >>= m_pImpl->m_sParam2;
            else if(rPropertyName == UNO_NAME_DATA_COLUMN_NAME)
                rValue >>= m_pImpl->m_sParam3;
            else if(rPropertyName == UNO_NAME_DATA_COMMAND_TYPE)
                rValue >>= m_pImpl->m_nParam2;
            if(rPropertyName == UNO_NAME_DATA_BASE_URL)
                rValue >>= m_pImpl->m_sParam5;

            if (  (   !m_pImpl->m_sParam1.isEmpty()
                   || !m_pImpl->m_sParam5.isEmpty())
                && !m_pImpl->m_sParam2.isEmpty()
                && !m_pImpl->m_sParam3.isEmpty())
            {
                GetFieldType();
            }
            break;
        case  RES_SETEXPFLD:
            if(rPropertyName == UNO_NAME_NUMBERING_SEPARATOR)
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName == UNO_NAME_CHAPTER_NUMBERING_LEVEL)
                rValue >>= m_pImpl->m_nParam1;
            break;
        case RES_DDEFLD:
            {
                sal_Int32 nPart = lcl_PropName2TokenPos(rPropertyName);
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
            throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        }
    }
}

SwFieldType* SwXFieldMaster::GetFieldType(bool const bDontCreate) const
{
#if !HAVE_FEATURE_DBCONNECTIVITY
    (void) bDontCreate;
#else
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
        SwFieldType *const pType = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
        pType->Add(m_pImpl.get());
        const_cast<SwXFieldMaster*>(this)->m_pImpl->m_bIsDescriptor = false;
    }
#endif
    if (m_pImpl->m_bIsDescriptor)
        return 0;
    else
        return static_cast<SwFieldType*>(m_pImpl->GetRegisteredIn());
}

uno::Any SAL_CALL
SwXFieldMaster::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFieldType* pType = GetFieldType(true);
    if( rPropertyName == UNO_NAME_INSTANCE_NAME )
    {
        OUString sName;
        if(pType)
            SwXTextFieldMasters::getInstanceName(*pType, sName);
        aRet <<= sName;
    }
    else if(pType)
    {
        if(rPropertyName == UNO_NAME_NAME)
        {
            aRet <<= SwXFieldMaster::GetProgrammaticName(*pType, *m_pImpl->m_pDoc);
        }
        else if(rPropertyName == UNO_NAME_DEPENDENT_TEXT_FIELDS)
        {
            //fill all text fields into a sequence
            std::vector<SwFormatField*>  aFieldArr;
            SwIterator<SwFormatField,SwFieldType> aIter( *pType );
            SwFormatField* pField = aIter.First();
            while(pField)
            {
                if(pField->IsFieldInDoc())
                    aFieldArr.push_back(pField);
                pField = aIter.Next();
            }

            uno::Sequence<uno::Reference <text::XDependentTextField> > aRetSeq(aFieldArr.size());
            uno::Reference<text::XDependentTextField>* pRetSeq = aRetSeq.getArray();
            for(size_t i = 0; i < aFieldArr.size(); ++i)
            {
                pField = aFieldArr[i];
                uno::Reference<text::XTextField> const xField =
                    SwXTextField::CreateXTextField(m_pImpl->m_pDoc, pField);

                pRetSeq[i] = uno::Reference<text::XDependentTextField>(xField,
                        uno::UNO_QUERY);
            }
            aRet <<= aRetSeq;
        }
        else
        {
            //TODO: add properties for the other field types
            const sal_uInt16 nMId = GetFieldTypeMId( rPropertyName, *pType );
            if (USHRT_MAX == nMId)
            {
                throw beans::UnknownPropertyException(
                        "Unknown property: " + rPropertyName,
                        static_cast<cppu::OWeakObject *>(this));
            }
            pType->QueryValue( aRet, nMId );

            if (rPropertyName == UNO_NAME_DATA_BASE_NAME ||
                rPropertyName == UNO_NAME_DATA_BASE_URL)
            {
                OUString aDataSource;
                aRet >>= aDataSource;
                aRet <<= OUString();

                OUString *pStr = 0;     // only one of this properties will return
                                        // a non-empty string.
                INetURLObject aObj;
                aObj.SetURL( aDataSource );
                bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
                if (bIsURL && rPropertyName == UNO_NAME_DATA_BASE_URL)
                    pStr = &aDataSource;        // DataBaseURL
                else if (!bIsURL && rPropertyName == UNO_NAME_DATA_BASE_NAME)
                    pStr = &aDataSource;        // DataBaseName

                if (pStr)
                    aRet <<= *pStr;
            }
        }
    }
    else
    {
        if(rPropertyName == UNO_NAME_DATA_COMMAND_TYPE)
            aRet <<= m_pImpl->m_nParam2;
        else if(rPropertyName == UNO_NAME_DEPENDENT_TEXT_FIELDS )
        {
            uno::Sequence<uno::Reference <text::XDependentTextField> > aRetSeq(0);
            aRet <<= aRetSeq;
        }
        else
        {
            switch (m_pImpl->m_nResTypeId)
            {
            case RES_USERFLD:
                if( rPropertyName == UNO_NAME_CONTENT )
                    aRet <<= m_pImpl->m_sParam1;
                else if(rPropertyName == UNO_NAME_VALUE)
                    aRet <<= m_pImpl->m_fParam1;
                else if(rPropertyName == UNO_NAME_IS_EXPRESSION)
                    aRet <<= m_pImpl->m_bParam1;
                break;
            case RES_DBFLD:
                if(rPropertyName == UNO_NAME_DATA_BASE_NAME ||
                   rPropertyName == UNO_NAME_DATA_BASE_URL)
                {
                     // only one of these properties returns a non-empty string.
                    INetURLObject aObj;
                    aObj.SetURL(m_pImpl->m_sParam5); // SetSmartURL
                    bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
                    if (bIsURL && rPropertyName == UNO_NAME_DATA_BASE_URL)
                        aRet <<= m_pImpl->m_sParam5; // DataBaseURL
                    else if ( rPropertyName == UNO_NAME_DATA_BASE_NAME)
                        aRet <<= m_pImpl->m_sParam1; // DataBaseName
                }
                else if(rPropertyName == UNO_NAME_DATA_TABLE_NAME)
                    aRet <<= m_pImpl->m_sParam2;
                else if(rPropertyName == UNO_NAME_DATA_COLUMN_NAME)
                    aRet <<= m_pImpl->m_sParam3;
                break;
            case RES_SETEXPFLD:
                if(rPropertyName == UNO_NAME_NUMBERING_SEPARATOR)
                    aRet <<= m_pImpl->m_sParam1;
                else if(rPropertyName == UNO_NAME_CHAPTER_NUMBERING_LEVEL)
                    aRet <<= m_pImpl->m_nParam1;
                break;
            case RES_DDEFLD:
                {
                    const sal_Int32 nPart = lcl_PropName2TokenPos(rPropertyName);
                    if(nPart  < 3 )
                        aRet <<= m_pImpl->m_sParam1.getToken(nPart, sfx2::cTokenSeparator);
                    else if(3 == nPart)
                        aRet <<= m_pImpl->m_bParam1;
                }
                break;
            default:
                throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            }
        }
    }
    return aRet;
}

void SwXFieldMaster::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXFieldMaster::dispose()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFieldType *const pFieldType = GetFieldType(true);
    if (!pFieldType)
        throw uno::RuntimeException();
    size_t nTypeIdx = SIZE_MAX;
    const SwFieldTypes* pTypes = m_pImpl->m_pDoc->getIDocumentFieldsAccess().GetFieldTypes();
    for( size_t i = 0; i < pTypes->size(); i++ )
    {
        if((*pTypes)[i] == pFieldType)
            nTypeIdx = i;
    }

    // first delete all fields
    SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
    SwFormatField* pField = aIter.First();
    while(pField)
    {
        SwTextField *pTextField = pField->GetTextField();
        if(pTextField && pTextField->GetTextNode().GetNodes().IsDocNodes() )
        {
            SwTextField::DeleteTextField(*pTextField);
        }
        pField = aIter.Next();
    }
    // then delete FieldType
    m_pImpl->m_pDoc->getIDocumentFieldsAccess().RemoveFieldType(nTypeIdx);
}

void SAL_CALL SwXFieldMaster::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXFieldMaster::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

void SwXFieldMaster::Impl::Modify(
        SfxPoolItem const*const pOld, SfxPoolItem const*const pNew)
{
    ClientModify(this, pOld, pNew);
    if (GetRegisteredIn())
    {
        return; // core object still alive
    }

    m_pDoc = 0;
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_EventListeners.disposeAndClear(ev);
}

OUString SwXFieldMaster::GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc)
{
    const OUString sName(rType.GetName());
    if(RES_SETEXPFLD == rType.Which())
    {
        const SwFieldTypes* pTypes = rDoc.getIDocumentFieldsAccess().GetFieldTypes();
        for( size_t i = 0; i <= size_t(INIT_FLDTYPES); i++ )
        {
            if((*pTypes)[i] == &rType)
            {
                return SwStyleNameMapper::GetProgName( sName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
            }
        }
    }
    return sName;
}

OUString SwXFieldMaster::LocalizeFormula(
    const SwSetExpField& rField,
    const OUString& rFormula,
    bool bQuery)
{
    const OUString sTypeName(rField.GetTyp()->GetName());
    const OUString sProgName(
        SwStyleNameMapper::GetProgName(sTypeName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL ));
    if(sProgName != sTypeName)
    {
        const OUString sSource = bQuery ? sTypeName : sProgName;
        const OUString sDest = bQuery ? sProgName : sTypeName;
        if(rFormula.startsWith(sSource))
        {
            return sDest + rFormula.copy(sSource.getLength());
        }
    }
    return rFormula;
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
    bool        bBool1;
    bool        bBool2;
    bool        bBool3;
    bool        bBool4;

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
        bBool1(false),
        bBool2(false),
        bBool3(false),
        bBool4(true) //Automatic language
        {}
    ~SwFieldProperties_Impl()
        {delete pDateTime;}

};

class SwXTextField::Impl
    : public SwClient
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::cppu::OInterfaceContainerHelper m_EventListeners;

    SwFormatField const*     m_pFormatField;
    SwDoc *             m_pDoc;
    SwTextAPIObject *   m_pTextObject;

    bool                m_bIsDescriptor;
    // required to access field master of not yet inserted fields
    SwClient            m_FieldTypeClient;
    bool                m_bCallUpdate;
    sal_uInt16          m_nServiceId;
    OUString            m_sTypeName;
    std::unique_ptr<SwFieldProperties_Impl> m_pProps;

    Impl(SwDoc *const pDoc, SwFormatField *const pFormat,
            sal_uInt16 const nServiceId)
        : SwClient(pFormat)
        , m_EventListeners(m_Mutex)
        , m_pFormatField(pFormat)
        , m_pDoc(pDoc)
        , m_pTextObject(0)
        , m_bIsDescriptor(pFormat == 0)
        , m_bCallUpdate(false)
        , m_nServiceId((pFormat)
                ? lcl_GetServiceForField(*pFormat->GetField())
                : nServiceId)
        , m_pProps((pFormat) ? 0 : new SwFieldProperties_Impl)
    { }

    virtual ~Impl()
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
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew) override;
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
throw (uno::RuntimeException, std::exception)
{
    return ::sw::UnoTunnelImpl<SwXTextField>(rId, this);
}

SwXTextField::SwXTextField(
    sal_uInt16 nServiceId,
    SwDoc* pDoc)
    : m_pImpl(new Impl(pDoc, 0, nServiceId))
{
    //Set visible as default!
    if ( SW_SERVICE_FIELDTYPE_SET_EXP == nServiceId
         || SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM == nServiceId
         || SW_SERVICE_FIELDTYPE_DATABASE == nServiceId
         || SW_SERVICE_FIELDTYPE_DATABASE_NAME == nServiceId )
    {
        m_pImpl->m_pProps->bBool2 = true;
    }
    else if(SW_SERVICE_FIELDTYPE_TABLE_FORMULA == nServiceId)
    {
        m_pImpl->m_pProps->bBool1 = true;
    }
    if(SW_SERVICE_FIELDTYPE_SET_EXP == nServiceId)
    {
        m_pImpl->m_pProps->nUSHORT2 = USHRT_MAX;
    }
}

SwXTextField::SwXTextField(SwFormatField& rFormat, SwDoc & rDoc)
    : m_pImpl(new Impl(&rDoc, &rFormat, USHRT_MAX))
{
}

SwXTextField::~SwXTextField()
{
}

uno::Reference<text::XTextField>
SwXTextField::CreateXTextField(SwDoc *const pDoc, SwFormatField const* pFormat,
        sal_uInt16 const nServiceId)
{
    assert(!pFormat || pDoc);
    assert(pFormat || nServiceId != 0xFFFF);
    // re-use existing SwXTextField
    uno::Reference<text::XTextField> xField;
    if (pFormat)
    {
        xField = pFormat->GetXTextField();
    }
    if (!xField.is())
    {
        SwXTextField *const pField( (pFormat)
                ? new SwXTextField(const_cast<SwFormatField&>(*pFormat), *pDoc)
                : new SwXTextField(nServiceId, pDoc));
        xField.set(pField);
        if (pFormat)
        {
            const_cast<SwFormatField *>(pFormat)->SetXTextField(xField);
        }
        // need a permanent Reference to initialize m_wThis
        pField->m_pImpl->m_wThis = xField;
    }
    return xField;
}

sal_uInt16 SwXTextField::GetServiceId() const
{
    return m_pImpl->m_nServiceId;
}

void SAL_CALL SwXTextField::attachTextFieldMaster(
        const uno::Reference< beans::XPropertySet > & xFieldMaster)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference< lang::XUnoTunnel > xMasterTunnel(xFieldMaster, uno::UNO_QUERY);
    if (!xMasterTunnel.is())
        throw lang::IllegalArgumentException();
    SwXFieldMaster* pMaster = reinterpret_cast< SwXFieldMaster * >(
            sal::static_int_cast< sal_IntPtr >( xMasterTunnel->getSomething( SwXFieldMaster::getUnoTunnelId()) ));

    SwFieldType* pFieldType = pMaster ? pMaster->GetFieldType() : 0;
    if (!pFieldType ||
        pFieldType->Which() != lcl_ServiceIdToResId(m_pImpl->m_nServiceId))
    {
        throw lang::IllegalArgumentException();
    }
    m_pImpl->m_sTypeName = pFieldType->GetName();
    pFieldType->Add( &m_pImpl->m_FieldTypeClient );
}

uno::Reference< beans::XPropertySet > SAL_CALL
SwXTextField::getTextFieldMaster() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFieldType* pType = 0;
    if (m_pImpl->m_bIsDescriptor && m_pImpl->m_FieldTypeClient.GetRegisteredIn())
    {
        pType = static_cast<SwFieldType*>(
                    m_pImpl->m_FieldTypeClient.GetRegisteredIn());
    }
    else
    {
        if (!m_pImpl->GetRegisteredIn())
            throw uno::RuntimeException();
        pType = m_pImpl->m_pFormatField->GetField()->GetTyp();
    }

    uno::Reference<beans::XPropertySet> const xRet(
            SwXFieldMaster::CreateXFieldMaster(m_pImpl->m_pDoc, pType));
    return xRet;
}

OUString SAL_CALL SwXTextField::getPresentation(sal_Bool bShowCommand)
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwField const*const pField = m_pImpl->GetField();
    if (!pField)
    {
        throw uno::RuntimeException();
    }
    return bShowCommand ? pField->GetFieldName() : pField->ExpandField(true);
}

void SAL_CALL SwXTextField::attach(
        const uno::Reference< text::XTextRange > & xTextRange)
throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (m_pImpl->m_bIsDescriptor)
    {
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

    SwDoc* pDoc = pRange ? &pRange->GetDoc() : pCursor ? pCursor->GetDoc() : 0;
    // if a FieldMaster was attached, then the document is already fixed!
    // NOTE: sw.SwXAutoTextEntry unoapi test depends on m_pDoc = 0 being valid
    if (!pDoc || (m_pImpl->m_pDoc && m_pImpl->m_pDoc != pDoc))
        throw lang::IllegalArgumentException();

    SwUnoInternalPaM aPam(*pDoc);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    SwField* pField = 0;
    switch (m_pImpl->m_nServiceId)
    {
        case SW_SERVICE_FIELDTYPE_ANNOTATION:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_POSTITFLD);

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
                SwPostItField* pPostItField = new SwPostItField(
                    static_cast<SwPostItFieldType*>(pFieldType),
                    m_pImpl->m_pProps->sPar1, // author
                    m_pImpl->m_pProps->sPar2, // content
                    m_pImpl->m_pProps->sPar3, // author's initials
                    m_pImpl->m_pProps->sPar4, // name
                    aDateTime );
                if ( m_pImpl->m_pTextObject )
                {
                    pPostItField->SetTextObject( m_pImpl->m_pTextObject->CreateText() );
                    pPostItField->SetPar2(m_pImpl->m_pTextObject->GetText());
                }
                pField = pPostItField;
            }
            break;
        case SW_SERVICE_FIELDTYPE_SCRIPT:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_SCRIPTFLD);
            pField = new SwScriptField(static_cast<SwScriptFieldType*>(pFieldType),
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
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DATETIMEFLD);
            SwDateTimeField *const pDTField = new SwDateTimeField(
                    static_cast<SwDateTimeFieldType*>(pFieldType),
                        nSub, m_pImpl->m_pProps->nFormat);
            pField = pDTField;
            if (m_pImpl->m_pProps->fDouble > 0.)
            {
                pDTField->SetValue(m_pImpl->m_pProps->fDouble);
            }
            if (m_pImpl->m_pProps->pDateTime)
            {
                uno::Any aVal; aVal <<= *m_pImpl->m_pProps->pDateTime;
                pField->PutValue( aVal, FIELD_PROP_DATE_TIME );
            }
            pDTField->SetOffset(m_pImpl->m_pProps->nSubType);
        }
        break;
        case SW_SERVICE_FIELDTYPE_FILE_NAME:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_FILENAMEFLD);
            sal_Int32 nFormat = m_pImpl->m_pProps->nFormat;
            if (m_pImpl->m_pProps->bBool2)
                nFormat |= FF_FIXED;
            SwFileNameField *const pFNField = new SwFileNameField(
                    static_cast<SwFileNameFieldType*>(pFieldType), nFormat);
            pField = pFNField;
            if (!m_pImpl->m_pProps->sPar3.isEmpty())
                pFNField->SetExpansion(m_pImpl->m_pProps->sPar3);
            uno::Any aFormat;
            aFormat <<= m_pImpl->m_pProps->nFormat;
            pField->PutValue( aFormat, FIELD_PROP_FORMAT );
        }
        break;
        case SW_SERVICE_FIELDTYPE_TEMPLATE_NAME:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_TEMPLNAMEFLD);
            pField = new SwTemplNameField(static_cast<SwTemplNameFieldType*>(pFieldType),
                                        m_pImpl->m_pProps->nFormat);
            uno::Any aFormat;
            aFormat <<= m_pImpl->m_pProps->nFormat;
            pField->PutValue(aFormat, FIELD_PROP_FORMAT);
        }
        break;
        case SW_SERVICE_FIELDTYPE_CHAPTER:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_CHAPTERFLD);
            SwChapterField *const pChapterField = new SwChapterField(
                    static_cast<SwChapterFieldType*>(pFieldType),
                    m_pImpl->m_pProps->nUSHORT1);
            pField = pChapterField;
            pChapterField->SetLevel(m_pImpl->m_pProps->nByte1);
            uno::Any aVal;
            aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
            pField->PutValue(aVal, FIELD_PROP_USHORT1 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_AUTHOR:
        {
            long nFormat = m_pImpl->m_pProps->bBool1 ? AF_NAME : AF_SHORTCUT;
            if (m_pImpl->m_pProps->bBool2)
                nFormat |= AF_FIXED;

            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_AUTHORFLD);
            SwAuthorField *const pAuthorField = new SwAuthorField(
                    static_cast<SwAuthorFieldType*>(pFieldType), nFormat);
            pField = pAuthorField;
            pAuthorField->SetExpansion(m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_CONDITIONED_TEXT:
        case SW_SERVICE_FIELDTYPE_HIDDEN_TEXT:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_HIDDENTXTFLD);
            SwHiddenTextField *const pHTField = new SwHiddenTextField(
                    static_cast<SwHiddenTextFieldType*>(pFieldType),
                    m_pImpl->m_pProps->sPar1,
                    m_pImpl->m_pProps->sPar2, m_pImpl->m_pProps->sPar3,
                    static_cast<sal_uInt16>(SW_SERVICE_FIELDTYPE_HIDDEN_TEXT == m_pImpl->m_nServiceId ?
                         TYP_HIDDENTXTFLD : TYP_CONDTXTFLD));
            pField = pHTField;
            pHTField->SetValue(m_pImpl->m_pProps->bBool1);
            uno::Any aVal;
            aVal <<= m_pImpl->m_pProps->sPar4;
            pField->PutValue(aVal, FIELD_PROP_PAR4 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_HIDDEN_PARA:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_HIDDENPARAFLD);
            SwHiddenParaField *const pHPField = new SwHiddenParaField(
                    static_cast<SwHiddenParaFieldType*>(pFieldType),
                    m_pImpl->m_pProps->sPar1);
            pField = pHPField;
            pHPField->SetHidden(m_pImpl->m_pProps->bBool1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_GET_REFERENCE:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_GETREFFLD);
            pField = new SwGetRefField(static_cast<SwGetRefFieldType*>(pFieldType),
                        m_pImpl->m_pProps->sPar1,
                        0,
                        0,
                        0);
            if (!m_pImpl->m_pProps->sPar3.isEmpty())
                static_cast<SwGetRefField*>(pField)->SetExpand(m_pImpl->m_pProps->sPar3);
            uno::Any aVal;
            aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
            pField->PutValue(aVal, FIELD_PROP_USHORT1 );
            aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT2);
            pField->PutValue(aVal, FIELD_PROP_USHORT2 );
            aVal <<= m_pImpl->m_pProps->nSHORT1;
            pField->PutValue(aVal, FIELD_PROP_SHORT1 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_JUMP_EDIT:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_JUMPEDITFLD);
            pField = new SwJumpEditField(static_cast<SwJumpEditFieldType*>(pFieldType),
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
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DOCINFOFLD);
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
            pField = new SwDocInfoField(
                    static_cast<SwDocInfoFieldType*>(pFieldType), nSubType,
                    m_pImpl->m_pProps->sPar4, m_pImpl->m_pProps->nFormat);
            if (!m_pImpl->m_pProps->sPar3.isEmpty())
                static_cast<SwDocInfoField*>(pField)->SetExpansion(m_pImpl->m_pProps->sPar3);
        }
        break;
        case SW_SERVICE_FIELDTYPE_USER_EXT:
        {
            sal_Int32 nFormat = 0;
            if (m_pImpl->m_pProps->bBool1)
                nFormat = AF_FIXED;

            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_EXTUSERFLD);
            SwExtUserField *const pEUField = new SwExtUserField(
                static_cast<SwExtUserFieldType*>(pFieldType),
                m_pImpl->m_pProps->nUSHORT1, nFormat);
            pField = pEUField;
            pEUField->SetExpansion(m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_USER:
        {
            SwFieldType* pFieldType =
                pDoc->getIDocumentFieldsAccess().GetFieldType(RES_USERFLD, m_pImpl->m_sTypeName, true);
            if (!pFieldType)
                throw uno::RuntimeException();
            sal_uInt16 nUserSubType = (m_pImpl->m_pProps->bBool1)
                ? nsSwExtendedSubType::SUB_INVISIBLE : 0;
            if (m_pImpl->m_pProps->bBool2)
                nUserSubType |= nsSwExtendedSubType::SUB_CMD;
            if (m_pImpl->m_pProps->bFormatIsDefault &&
                nsSwGetSetExpType::GSE_STRING == static_cast<SwUserFieldType*>(pFieldType)->GetType())
            {
                m_pImpl->m_pProps->nFormat = -1;
            }
            pField = new SwUserField(static_cast<SwUserFieldType*>(pFieldType),
                                nUserSubType,
                                m_pImpl->m_pProps->nFormat);
        }
        break;
        case SW_SERVICE_FIELDTYPE_REF_PAGE_SET:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_REFPAGESETFLD);
            pField = new SwRefPageSetField( static_cast<SwRefPageSetFieldType*>(pFieldType),
                                m_pImpl->m_pProps->nUSHORT1,
                                m_pImpl->m_pProps->bBool1 );
        }
        break;
        case SW_SERVICE_FIELDTYPE_REF_PAGE_GET:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_REFPAGEGETFLD);
            SwRefPageGetField *const pRGField = new SwRefPageGetField(
                    static_cast<SwRefPageGetFieldType*>(pFieldType),
                    m_pImpl->m_pProps->nUSHORT1 );
            pField = pRGField;
            pRGField->SetText(m_pImpl->m_pProps->sPar1);
        }
        break;
        case SW_SERVICE_FIELDTYPE_PAGE_NUM:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_PAGENUMBERFLD);
            SwPageNumberField *const pPNField = new SwPageNumberField(
                static_cast<SwPageNumberFieldType*>(pFieldType), PG_RANDOM,
                m_pImpl->m_pProps->nFormat,
                m_pImpl->m_pProps->nUSHORT1);
            pField = pPNField;
            pPNField->SetUserString(m_pImpl->m_pProps->sPar1);
            uno::Any aVal;
            aVal <<= m_pImpl->m_pProps->nSubType;
            pField->PutValue( aVal, FIELD_PROP_SUBTYPE );
        }
        break;
        case SW_SERVICE_FIELDTYPE_DDE:
        {
            SwFieldType* pFieldType =
                pDoc->getIDocumentFieldsAccess().GetFieldType(RES_DDEFLD, m_pImpl->m_sTypeName, true);
            if (!pFieldType)
                throw uno::RuntimeException();
            pField = new SwDDEField( static_cast<SwDDEFieldType*>(pFieldType) );
        }
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NAME:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DBNAMEFLD);
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            pField = new SwDBNameField(static_cast<SwDBNameFieldType*>(pFieldType), aData);
            sal_uInt16  nSubType = pField->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            pField->SetSubType(nSubType);
        }
#endif
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NEXT_SET:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DBNEXTSETFLD);
            pField = new SwDBNextSetField(static_cast<SwDBNextSetFieldType*>(pFieldType),
                    m_pImpl->m_pProps->sPar3, OUString(), aData);
        }
#endif
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_NUM_SET:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            pField = new SwDBNumSetField( static_cast<SwDBNumSetFieldType*>(
                pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DBNUMSETFLD)),
                m_pImpl->m_pProps->sPar3,
                OUString::number(m_pImpl->m_pProps->nFormat),
                aData );
        }
#endif
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE_SET_NUM:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            SwDBData aData;
            aData.sDataSource = m_pImpl->m_pProps->sPar1;
            aData.sCommand = m_pImpl->m_pProps->sPar2;
            aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
            SwDBSetNumberField *const pDBSNField =
                new SwDBSetNumberField(static_cast<SwDBSetNumberFieldType*>(
                        pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DBSETNUMBERFLD)), aData,
                    m_pImpl->m_pProps->nUSHORT1);
            pField = pDBSNField;
            pDBSNField->SetSetNumber(m_pImpl->m_pProps->nFormat);
            sal_uInt16  nSubType = pField->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            pField->SetSubType(nSubType);
        }
#endif
        break;
        case SW_SERVICE_FIELDTYPE_DATABASE:
#if HAVE_FEATURE_DBCONNECTIVITY
        {
            SwFieldType* pFieldType =
                pDoc->getIDocumentFieldsAccess().GetFieldType(RES_DBFLD, m_pImpl->m_sTypeName, false);
            if (!pFieldType)
                throw uno::RuntimeException();
            pField = new SwDBField(static_cast<SwDBFieldType*>(pFieldType),
                    m_pImpl->m_pProps->nFormat);
            static_cast<SwDBField*>(pField)->InitContent(m_pImpl->m_pProps->sPar1);
            sal_uInt16  nSubType = pField->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            pField->SetSubType(nSubType);
        }
#endif
        break;
        case SW_SERVICE_FIELDTYPE_SET_EXP:
        {
            SwFieldType* pFieldType =
                pDoc->getIDocumentFieldsAccess().GetFieldType(RES_SETEXPFLD, m_pImpl->m_sTypeName, true);
            if (!pFieldType)
                throw uno::RuntimeException();
            // detect the field type's sub type and set an appropriate number format
            if (m_pImpl->m_pProps->bFormatIsDefault &&
                nsSwGetSetExpType::GSE_STRING == static_cast<SwSetExpFieldType*>(pFieldType)->GetType())
            {
                m_pImpl->m_pProps->nFormat = -1;
            }
            SwSetExpField *const pSEField = new SwSetExpField(
                static_cast<SwSetExpFieldType*>(pFieldType),
                m_pImpl->m_pProps->sPar2,
                m_pImpl->m_pProps->nUSHORT2 != USHRT_MAX ?  //#i79471# the field can have a number format or a number_ing_ format
                m_pImpl->m_pProps->nUSHORT2 : m_pImpl->m_pProps->nFormat);
            pField = pSEField;

            sal_uInt16  nSubType = pField->GetSubType();
            if (m_pImpl->m_pProps->bBool2)
                nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
            else
                nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
            if (m_pImpl->m_pProps->bBool3)
                nSubType |= nsSwExtendedSubType::SUB_CMD;
            else
                nSubType &= ~nsSwExtendedSubType::SUB_CMD;
            pField->SetSubType(nSubType);
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
            SwFieldType* pSetExpField = pDoc->getIDocumentFieldsAccess().GetFieldType(
                    RES_SETEXPFLD, m_pImpl->m_pProps->sPar1, false);
            bool bSetGetExpFieldUninitialized = false;
            if (pSetExpField)
            {
                if (nSubType != nsSwGetSetExpType::GSE_STRING &&
                    static_cast< SwSetExpFieldType* >(pSetExpField)->GetType() == nsSwGetSetExpType::GSE_STRING)
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
                        pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_GETEXPFLD)),
                    m_pImpl->m_pProps->sPar1, nSubType,
                    m_pImpl->m_pProps->nFormat);
            pField = pGEField;
            //TODO: evaluate SubType!
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
            SwFieldType* pFieldType =
                pDoc->getIDocumentFieldsAccess().GetFieldType(RES_INPUTFLD, m_pImpl->m_sTypeName, true);
            if (!pFieldType)
                throw uno::RuntimeException();
            sal_uInt16 nInpSubType =
                sal::static_int_cast<sal_uInt16>(
                    SW_SERVICE_FIELDTYPE_INPUT_USER == m_pImpl->m_nServiceId
                        ? INP_USR : INP_TXT);
            SwInputField * pTextField =
                new SwInputField(static_cast<SwInputFieldType*>(pFieldType),
                                 m_pImpl->m_pProps->sPar1,
                                 m_pImpl->m_pProps->sPar2,
                                 nInpSubType);
            pTextField->SetHelp(m_pImpl->m_pProps->sPar3);
            pTextField->SetToolTip(m_pImpl->m_pProps->sPar4);

            pField = pTextField;
        }
        break;
        case SW_SERVICE_FIELDTYPE_MACRO:
        {
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_MACROFLD);
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
            pField = new SwMacroField(static_cast<SwMacroFieldType*>(pFieldType), aName,
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
                case SW_SERVICE_FIELDTYPE_PARAGRAPH_COUNT       : nSubType = DS_PARA; break;
                case SW_SERVICE_FIELDTYPE_WORD_COUNT            : nSubType = DS_WORD; break;
                case SW_SERVICE_FIELDTYPE_CHARACTER_COUNT       : nSubType = DS_CHAR; break;
                case SW_SERVICE_FIELDTYPE_TABLE_COUNT           : nSubType = DS_TBL;  break;
                case SW_SERVICE_FIELDTYPE_GRAPHIC_OBJECT_COUNT  : nSubType = DS_GRF;  break;
                case SW_SERVICE_FIELDTYPE_EMBEDDED_OBJECT_COUNT : nSubType = DS_OLE;  break;
            }
            SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DOCSTATFLD);
            pField = new SwDocStatField(
                    static_cast<SwDocStatFieldType*>(pFieldType),
                    nSubType, m_pImpl->m_pProps->nUSHORT2);
        }
        break;
        case SW_SERVICE_FIELDTYPE_BIBLIOGRAPHY:
        {
            SwAuthorityFieldType const type(pDoc);
            pField = new SwAuthorityField(static_cast<SwAuthorityFieldType*>(
                        pDoc->getIDocumentFieldsAccess().InsertFieldType(type)),
                    OUString());
            if (m_pImpl->m_pProps->aPropSeq.getLength())
            {
                uno::Any aVal;
                aVal <<= m_pImpl->m_pProps->aPropSeq;
                pField->PutValue( aVal, FIELD_PROP_PROP_SEQ );
            }
        }
        break;
        case SW_SERVICE_FIELDTYPE_COMBINED_CHARACTERS:
            // create field
            pField = new SwCombinedCharField( static_cast<SwCombinedCharFieldType*>(
                        pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_COMBINED_CHARS)),
                        m_pImpl->m_pProps->sPar1);
            break;
        case SW_SERVICE_FIELDTYPE_DROPDOWN:
        {
            SwDropDownField *const pDDField = new SwDropDownField(
                static_cast<SwDropDownFieldType *>(
                    pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_DROPDOWN)));
            pField = pDDField;

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
            pField = new SwTableField( static_cast<SwTableFieldType*>(
                pDoc->getIDocumentFieldsAccess().GetSysFieldType(RES_TABLEFLD)),
                m_pImpl->m_pProps->sPar2,
                nType,
                m_pImpl->m_pProps->nFormat);
           static_cast<SwTableField*>(pField)->ChgExpStr(m_pImpl->m_pProps->sPar1);
        }
        break;
        default: OSL_FAIL("was ist das fuer ein Typ?");
    }
    if (!pField)
        throw uno::RuntimeException("no SwField created?");
    if (pField)
    {
        pField->SetAutomaticLanguage(!m_pImpl->m_pProps->bBool4);
        SwFormatField aFormat( *pField );

        UnoActionContext aCont(pDoc);
        if (aPam.HasMark() &&
            m_pImpl->m_nServiceId != SW_SERVICE_FIELDTYPE_ANNOTATION)
        {
            pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
        }

        SwXTextCursor const*const pTextCursor(dynamic_cast<SwXTextCursor*>(pCursor));
        const bool bForceExpandHints(
            pTextCursor
            && pTextCursor->IsAtEndOfMeta() );
        const SetAttrMode nInsertFlags =
            (bForceExpandHints)
            ? SetAttrMode::FORCEHINTEXPAND
            : SetAttrMode::DEFAULT;

        if (*aPam.GetPoint() != *aPam.GetMark() &&
            m_pImpl->m_nServiceId == SW_SERVICE_FIELDTYPE_ANNOTATION)
        {
            // Make sure we always insert the field at the end
            SwPaM aEnd(*aPam.End(), *aPam.End());
            pDoc->getIDocumentContentOperations().InsertPoolItem(aEnd, aFormat, nInsertFlags);
        }
        else
            pDoc->getIDocumentContentOperations().InsertPoolItem(aPam, aFormat, nInsertFlags);

        SwTextAttr* pTextAttr = aPam.GetNode().GetTextNode()->GetFieldTextAttrAt( aPam.GetPoint()->nContent.GetIndex()-1, true );

        // What about updating the fields? (see fldmgr.cxx)
        if (pTextAttr)
        {
            const SwFormatField& rField = pTextAttr->GetFormatField();
            m_pImpl->m_pFormatField = &rField;

            if ( pTextAttr->Which() == RES_TXTATR_ANNOTATION
                 && *aPam.GetPoint() != *aPam.GetMark() )
            {
                // create annotation mark
                const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pTextAttr->GetFormatField().GetField());
                OSL_ENSURE( pPostItField != NULL, "<SwXTextField::attachToRange(..)> - annotation field missing!" );
                if ( pPostItField != NULL )
                {
                    IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
                    pMarksAccess->makeAnnotationMark( aPam, pPostItField->GetName() );
                }
            }
        }
        else // could theoretically happen, if paragraph is full
            throw uno::RuntimeException("no SwTextAttr inserted?");
    }
    delete pField;

    assert(m_pImpl->m_pFormatField);
    m_pImpl->m_pDoc = pDoc;
    const_cast<SwFormatField *>(m_pImpl->m_pFormatField)->Add(m_pImpl.get());
    m_pImpl->m_bIsDescriptor = false;
    if (m_pImpl->m_FieldTypeClient.GetRegisteredIn())
    {
        m_pImpl->m_FieldTypeClient.GetRegisteredIn()
            ->Remove(&m_pImpl->m_FieldTypeClient);
    }
    m_pImpl->m_pProps.reset();
    if (m_pImpl->m_bCallUpdate)
        update();
    }
    else if ( m_pImpl->m_pFormatField != NULL
              && m_pImpl->m_pDoc != NULL
              && m_pImpl->m_nServiceId == SW_SERVICE_FIELDTYPE_ANNOTATION )
    {
        SwUnoInternalPaM aIntPam( *m_pImpl->m_pDoc );
        if ( ::sw::XTextRangeToSwPaM( aIntPam, xTextRange ) )
        {
            // nothing to do, if the text range only covers the former annotation field
            if ( aIntPam.Start()->nNode != aIntPam.End()->nNode
                 || aIntPam.Start()->nContent.GetIndex() != aIntPam.End()->nContent.GetIndex()-1 )
            {
                UnoActionContext aCont( m_pImpl->m_pDoc );
                // insert copy of annotation at new text range
                SwPostItField* pPostItField = static_cast< SwPostItField* >(m_pImpl->m_pFormatField->GetField()->CopyField());
                SwFormatField aFormatField( *pPostItField );
                delete pPostItField;
                SwPaM aEnd( *aIntPam.End(), *aIntPam.End() );
                m_pImpl->m_pDoc->getIDocumentContentOperations().InsertPoolItem( aEnd, aFormatField );
                // delete former annotation
                {
                    const SwTextField* pTextField = m_pImpl->m_pFormatField->GetTextField();
                    SwTextNode& rTextNode = (SwTextNode&)*pTextField->GetpTextNode();
                    SwPaM aPam( rTextNode, pTextField->GetStart() );
                    aPam.SetMark();
                    aPam.Move();
                    m_pImpl->m_pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
                }
                // keep inserted annotation
                {
                    SwTextField* pTextAttr = aEnd.GetNode().GetTextNode()->GetFieldTextAttrAt( aEnd.End()->nContent.GetIndex()-1, true );
                    if ( pTextAttr != NULL )
                    {
                        m_pImpl->m_pFormatField = &pTextAttr->GetFormatField();

                        if ( *aIntPam.GetPoint() != *aIntPam.GetMark() )
                        {
                            // create annotation mark
                            const SwPostItField* pField = dynamic_cast< const SwPostItField* >(pTextAttr->GetFormatField().GetField());
                            OSL_ENSURE( pField != NULL, "<SwXTextField::attach(..)> - annotation field missing!" );
                            if ( pField != NULL )
                            {
                                IDocumentMarkAccess* pMarksAccess = aIntPam.GetDoc()->getIDocumentMarkAccess();
                                pMarksAccess->makeAnnotationMark( aIntPam, pField->GetName() );
                            }
                        }
                    }
                }
            }
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw lang::IllegalArgumentException();
}

uno::Reference< text::XTextRange > SAL_CALL
SwXTextField::getAnchor() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwField const*const pField = m_pImpl->GetField();
    if (!pField)
        return 0;

    const SwTextField* pTextField = m_pImpl->m_pFormatField->GetTextField();
    if (!pTextField)
        throw uno::RuntimeException();

    std::shared_ptr< SwPaM > pPamForTextField;
    SwTextField::GetPamForTextField(*pTextField, pPamForTextField);
    if (pPamForTextField.get() == NULL)
        return 0;

    // If this is a postit field, then return the range of its annotation mark if it has one.
    if (pField->Which() == RES_POSTITFLD)
    {
        const SwPostItField* pPostItField = static_cast<const SwPostItField*>(pField);
        IDocumentMarkAccess* pMarkAccess = m_pImpl->m_pDoc->getIDocumentMarkAccess();
        for (IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAnnotationMarksBegin(); ppMark != pMarkAccess->getAnnotationMarksEnd(); ++ppMark)
        {
            if (ppMark->get()->GetName() == pPostItField->GetName())
            {
                pPamForTextField.reset(new SwPaM(ppMark->get()->GetMarkStart(), ppMark->get()->GetMarkEnd()));
                break;
            }
        }
    }

    uno::Reference<text::XTextRange> xRange = SwXTextRange::CreateXTextRange(
            *m_pImpl->m_pDoc, *(pPamForTextField->GetPoint()), pPamForTextField->GetMark());
    return xRange;
}

void SAL_CALL SwXTextField::dispose() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwField const*const pField = m_pImpl->GetField();
    if(pField)
    {
        UnoActionContext aContext(m_pImpl->m_pDoc);

        assert(m_pImpl->m_pFormatField->GetTextField() && "<SwXTextField::dispose()> - missing <SwTextField> --> crash");
        SwTextField::DeleteTextField(*(m_pImpl->m_pFormatField->GetTextField()));
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
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXTextField::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextField::getPropertySetInfo()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    // no static
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
    uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwField const*const pField = m_pImpl->GetField();
    const SfxItemPropertySet* _pPropSet = aSwMapProvider.GetPropertySet(
                lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    const SfxItemPropertySimpleEntry*   pEntry = _pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
        throw beans::PropertyVetoException( "Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pField)
    {
        // special treatment for mail merge fields
        const sal_uInt16 nWhich = pField->Which();
        if( RES_DBFLD == nWhich &&
            (rPropertyName == UNO_NAME_DATA_BASE_NAME ||
            rPropertyName == UNO_NAME_DATA_BASE_URL||
            rPropertyName == UNO_NAME_DATA_TABLE_NAME||
            rPropertyName == UNO_NAME_DATA_COLUMN_NAME))
        {
            // here a new field type must be created and the field must
            // be registered at the new type
            OSL_FAIL("not implemented");
        }
        else
        {
            SwDoc * pDoc = m_pImpl->m_pDoc;
            assert(pDoc);
            const SwTextField* pTextField = m_pImpl->m_pFormatField->GetTextField();
            if(!pTextField)
                throw uno::RuntimeException();
            SwPosition aPosition( pTextField->GetTextNode() );
            aPosition.nContent = pTextField->GetStart();
            pDoc->getIDocumentFieldsAccess().PutValueToField( aPosition, rValue, pEntry->nWID);
        }

        //#i100374# notify SwPostIt about new field content
        if (RES_POSTITFLD == nWhich && m_pImpl->m_pFormatField)
        {
            const_cast<SwFormatField*>(m_pImpl->m_pFormatField)->Broadcast(
                    SwFormatFieldHint( 0, SwFormatFieldHintWhich::CHANGED ));
        }

        // fdo#42073 notify SwTextField about changes of the expanded string
        if (m_pImpl->m_pFormatField->GetTextField())
        {
            m_pImpl->m_pFormatField->GetTextField()->ExpandTextField();
        }

        //#i100374# changing a document field should set the modify flag
        SwDoc* pDoc = m_pImpl->m_pDoc;
        if (pDoc)
            pDoc->getIDocumentState().SetModified();

    }
    else if (m_pImpl->m_pProps)
    {
        bool* pBool = 0;
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
            if(rValue.getValueType() != ::cppu::UnoType<util::Date>::get())
                throw lang::IllegalArgumentException();

            util::Date aTemp = *static_cast<const util::Date*>(rValue.getValue());
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
            if(rValue.getValueType() != ::cppu::UnoType<double>::get())
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
            if( rValue.getValueType() == cppu::UnoType<bool>::get() )
                *pBool = *static_cast<sal_Bool const *>(rValue.getValue());
            else
                throw lang::IllegalArgumentException();
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL SwXTextField::getPropertyValue(const OUString& rPropertyName)
throw (beans::UnknownPropertyException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
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
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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
            aRet.setValue(&aTypes, cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get());
        }
        break;

    default:
        if( pField )
        {
            if (FIELD_PROP_IS_FIELD_USED      == pEntry->nWID ||
                FIELD_PROP_IS_FIELD_DISPLAYED == pEntry->nWID)
            {
                bool bIsFieldUsed       = false;
                bool bIsFieldDisplayed  = false;

                // in order to have the information about fields
                // correctly evaluated the document needs a layout
                // (has to be already formatted)
                SwDoc *pDoc = m_pImpl->m_pDoc;
                SwViewShell *pViewShell = 0;
                SwEditShell *pEditShell = 0;
                if( pDoc )
                {
                    pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
                    pEditShell = pDoc->GetEditShell();
                }

                if (pEditShell)
                    pEditShell->CalcLayout();
                else if (pViewShell) // a page preview has no SwEditShell it should only have a view shell
                    pViewShell->CalcLayout();
                else
                    throw uno::RuntimeException();

                // get text node for the text field
                const SwFormatField *pFieldFormat =
                    (m_pImpl->GetField()) ? m_pImpl->m_pFormatField : 0;
                const SwTextField* pTextField = (pFieldFormat)
                    ? m_pImpl->m_pFormatField->GetTextField() : 0;
                if(!pTextField)
                    throw uno::RuntimeException();
                const SwTextNode& rTextNode = pTextField->GetTextNode();

                // skip fields that are currently not in the document
                // e.g. fields in undo or redo array
                if (rTextNode.GetNodes().IsDocNodes())
                {
                    bool bFrame = 0 != rTextNode.FindLayoutRect().Width(); // oder so
                    bool bHidden = rTextNode.IsHidden();
                    if ( !bHidden )
                    {
                        sal_Int32 nHiddenStart;
                        sal_Int32 nHiddenEnd;

                        SwPosition aPosition( pTextField->GetTextNode() );
                        aPosition.nContent = pTextField->GetStart();

                        bHidden = SwScriptInfo::GetBoundsOfHiddenRange( pTextField->GetTextNode(),
                                        pTextField->GetStart(),
                                        nHiddenStart, nHiddenEnd );
                    }

                    // !bFrame && !bHidden: most probably a field in an unused page style

                    // FME: Problem: hidden field in unused page template =>
                    // bIsFieldUsed = true
                    // bIsFieldDisplayed = false
                    bIsFieldUsed       = bFrame || bHidden;
                    bIsFieldDisplayed  = bIsFieldUsed && !bHidden;
                }
                aRet <<= (FIELD_PROP_IS_FIELD_USED == pEntry->nWID) ? bIsFieldUsed : bIsFieldDisplayed;
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
                aRet <<= m_pImpl->m_pProps->bBool1;
                break;
            case FIELD_PROP_BOOL2 :
                aRet <<= m_pImpl->m_pProps->bBool2;
                break;
            case FIELD_PROP_BOOL3 :
                aRet <<= m_pImpl->m_pProps->bBool3;
                break;
            case FIELD_PROP_BOOL4 :
                aRet <<= m_pImpl->m_pProps->bBool4;
                break;
            case FIELD_PROP_DATE :
                aRet.setValue(&m_pImpl->m_pProps->aDate, ::cppu::UnoType<util::Date>::get());
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

void SwXTextField::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextField::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextField::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextField::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextField::update() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwField * pField = const_cast<SwField*>(m_pImpl->GetField());
    if (pField)
    {
        switch(pField->Which())
        {
            case RES_DATETIMEFLD:
            static_cast<SwDateTimeField*>(pField)->SetDateTime( ::DateTime( ::DateTime::SYSTEM ) );
            break;

            case RES_EXTUSERFLD:
            {
                SwExtUserField* pExtUserField = static_cast<SwExtUserField*>(pField);
                pExtUserField->SetExpansion( SwExtUserFieldType::Expand(
                                            pExtUserField->GetSubType(),
                                            pExtUserField->GetFormat() ) );
            }
            break;

            case RES_AUTHORFLD:
            {
                SwAuthorField* pAuthorField = static_cast<SwAuthorField*>(pField);
                pAuthorField->SetExpansion( SwAuthorFieldType::Expand(
                                            pAuthorField->GetFormat() ) );
            }
            break;

            case RES_FILENAMEFLD:
            {
                SwFileNameField* pFileNameField = static_cast<SwFileNameField*>(pField);
                pFileNameField->SetExpansion( static_cast<SwFileNameFieldType*>(pField->GetTyp())->Expand(
                                            pFileNameField->GetFormat() ) );
            }
            break;

            case RES_DOCINFOFLD:
            {
                    SwDocInfoField* pDocInfField = static_cast<SwDocInfoField*>(pField);
                    pDocInfField->SetExpansion( static_cast<SwDocInfoFieldType*>(pField->GetTyp())->Expand(
                                                pDocInfField->GetSubType(),
                                                pDocInfField->GetFormat(),
                                                pDocInfField->GetLanguage(),
                                                pDocInfField->GetName() ) );
            }
            break;
        }
        // Text formatting has to be triggered.
        const_cast<SwFormatField*>(m_pImpl->m_pFormatField)->ModifyNotification(0, 0);
    }
    else
        m_pImpl->m_bCallUpdate = true;
}

OUString SAL_CALL SwXTextField::getImplementationName()
throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXTextField");
}

static OUString OldNameToNewName_Impl( const OUString &rOld )
{
    static const char aOldNamePart1[] = ".TextField.DocInfo.";
    static const char aOldNamePart2[] = ".TextField.";
    static const char aNewNamePart1[] = ".textfield.docinfo.";
    static const char aNewNamePart2[] = ".textfield.";
    OUString sServiceNameCC( rOld );
    sal_Int32 nIdx = sServiceNameCC.indexOf( aOldNamePart1 );
    if (nIdx >= 0)
        sServiceNameCC = sServiceNameCC.replaceAt( nIdx, strlen(aOldNamePart1), aNewNamePart1 );
    nIdx = sServiceNameCC.indexOf( aOldNamePart2 );
    if (nIdx >= 0)
        sServiceNameCC = sServiceNameCC.replaceAt( nIdx, strlen(aOldNamePart2), aNewNamePart2 );
    return sServiceNameCC;
}

sal_Bool SAL_CALL SwXTextField::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwXTextField::getSupportedServiceNames()
throw (uno::RuntimeException, std::exception)
{
    const OUString sServiceName =
        SwXServiceProvider::GetProviderName(m_pImpl->m_nServiceId);

    // case-corrected version of service-name (see #i67811)
    // (need to supply both because of compatibility to older versions)
    const OUString sServiceNameCC(  OldNameToNewName_Impl( sServiceName ) );
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
        m_pFormatField = 0;
        m_pDoc = 0;
        uno::Reference<uno::XInterface> const xThis(m_wThis);
        if (!xThis.is())
        {   // fdo#72695: if UNO object is already dead, don't revive it with event
            return;
        }
        lang::EventObject const ev(xThis);
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
        if( static_cast<void*>(GetRegisteredIn()) == static_cast<const SwPtrMsgPoolItem *>(pOld)->pObject )
            Invalidate();
        break;

    case RES_FMT_CHG:
        // Am I re-attached to a new one and will the old one be deleted?
        if( static_cast<const SwFormatChg*>(pNew)->pChangedFormat == GetRegisteredIn() &&
            static_cast<const SwFormatChg*>(pOld)->pChangedFormat->IsFormatInDTOR() )
            Invalidate();
        break;
    }
}

const SwField*  SwXTextField::Impl::GetField() const
{
    if (GetRegisteredIn() && m_pFormatField)
    {
        return m_pFormatField->GetField();
    }
    return 0;
}

OUString SwXTextFieldMasters::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextFieldMasters");
}

sal_Bool SwXTextFieldMasters::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextFieldMasters::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
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

/*
    Iteration over non-standard field types
    USER/SETEXP/DDE/DATABASE
    Thus the names are:
    "com.sun.star.text.fieldmaster.User" + <field type name>
    "com.sun.star.text.fieldmaster.DDE" + <field type name>
    "com.sun.star.text.fieldmaster.SetExpression" + <field type name>
    "com.sun.star.text.fieldmaster.DataBase" + <field type name>

    If too much, maybe one could leave out the "com.sun.star.text".
 */
static sal_uInt16 lcl_GetIdByName( OUString& rName, OUString& rTypeName )
{
    if (rName.startsWithIgnoreAsciiCase(COM_TEXT_FLDMASTER_CC))
        rName = rName.copy(30);

    sal_uInt16 nResId = USHRT_MAX;
    sal_Int32 nFound = 0;
    rTypeName = rName.getToken( 0, '.', nFound );
    if (rTypeName == "User")
        nResId = RES_USERFLD;
    else if (rTypeName == "DDE")
        nResId = RES_DDEFLD;
    else if (rTypeName == "SetExpression")
    {
        nResId = RES_SETEXPFLD;

        const OUString sFieldTypName( rName.getToken( 1, '.' ));
        const OUString sUIName( SwStyleNameMapper::GetSpecialExtraUIName( sFieldTypName ) );

        if( sUIName != sFieldTypName )
            rName = comphelper::string::setToken(rName, 1, '.', sUIName);
    }
    else if (rTypeName.equalsIgnoreAsciiCase("DataBase"))
    {
        rName = rName.copy(RTL_CONSTASCII_LENGTH("DataBase."));
        const sal_Int32 nDotCount = comphelper::string::getTokenCount(rName, '.');
        if( 1 <= nDotCount )
        {
            // #i51815#
            rName = "DataBase." + rName;
            nResId = RES_DBFLD;
        }
    }
    else if (rTypeName == "Bibliography")
        nResId = RES_AUTHORITY;
    return nResId;
}

uno::Any SwXTextFieldMasters::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    OUString sName(rName), sTypeName;
    const sal_uInt16 nResId = lcl_GetIdByName( sName, sTypeName );
    if( USHRT_MAX == nResId )
        throw container::NoSuchElementException(
            "SwXTextFieldMasters::getByName(" + rName + ")",
            css::uno::Reference<css::uno::XInterface>());

    sName = sName.copy(std::min(sTypeName.getLength()+1, sName.getLength()));
    SwFieldType* pType = GetDoc()->getIDocumentFieldsAccess().GetFieldType(nResId, sName, true);
    if(!pType)
        throw container::NoSuchElementException(
            "SwXTextFieldMasters::getByName(" + rName + ")",
            css::uno::Reference<css::uno::XInterface>());

    uno::Reference<beans::XPropertySet> const xRet(
            SwXFieldMaster::CreateXFieldMaster(GetDoc(), pType));
    return uno::makeAny(xRet);
}

bool SwXTextFieldMasters::getInstanceName(
    const SwFieldType& rFieldType, OUString& rName)
{
    OUString sField;

    switch( rFieldType.Which() )
    {
    case RES_USERFLD:
        sField = "User." + rFieldType.GetName();
        break;
    case RES_DDEFLD:
        sField = "DDE." + rFieldType.GetName();
        break;

    case RES_SETEXPFLD:
        sField = "SetExpression." + SwStyleNameMapper::GetSpecialExtraProgName( rFieldType.GetName() );
        break;

    case RES_DBFLD:
        sField = "DataBase." + rFieldType.GetName().replaceAll(OUString(DB_DELIM), ".");
        break;

    case RES_AUTHORITY:
        sField = "Bibliography";
        break;

    default:
        return false;
    }

    rName += COM_TEXT_FLDMASTER_CC + sField;
    return true;
}

uno::Sequence< OUString > SwXTextFieldMasters::getElementNames()
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    const SwFieldTypes* pFieldTypes = GetDoc()->getIDocumentFieldsAccess().GetFieldTypes();
    const size_t nCount = pFieldTypes->size();

    std::vector<OUString> aFieldNames;
    for( size_t i = 0; i < nCount; ++i )
    {
        SwFieldType& rFieldType = *((*pFieldTypes)[i]);

        OUString sFieldName;
        if (SwXTextFieldMasters::getInstanceName(rFieldType, sFieldName))
        {
            aFieldNames.push_back(sFieldName);
        }
    }

    uno::Sequence< OUString > aSeq( static_cast<sal_Int32>(aFieldNames.size()) );
    OUString* pArray = aSeq.getArray();
    for (size_t i = 0; i<aFieldNames.size(); ++i)
    {
        pArray[i] = aFieldNames[i];
    }

    return aSeq;
}

sal_Bool SwXTextFieldMasters::hasByName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    OUString sName(rName), sTypeName;
    const sal_uInt16 nResId = lcl_GetIdByName( sName, sTypeName );
    bool bRet = false;
    if( USHRT_MAX != nResId )
    {
        sName = sName.copy(std::min(sTypeName.getLength()+1, sName.getLength()));
        bRet = USHRT_MAX != nResId && 0 != GetDoc()->getIDocumentFieldsAccess().GetFieldType(nResId, sName, true);
    }
    return bRet;
}

uno::Type  SwXTextFieldMasters::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<beans::XPropertySet>::get();

}

sal_Bool SwXTextFieldMasters::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return sal_True;
}

class SwXTextFieldTypes::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    ::cppu::OInterfaceContainerHelper m_RefreshListeners;

    Impl() : m_RefreshListeners(m_Mutex) { }
};

OUString SwXTextFieldTypes::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextFieldTypes");
}

sal_Bool SwXTextFieldTypes::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextFieldTypes::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
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

uno::Reference< container::XEnumeration >  SwXTextFieldTypes::createEnumeration()
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return new SwXFieldEnumeration(*GetDoc());
}

uno::Type  SwXTextFieldTypes::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<text::XDependentTextField>::get();
}

sal_Bool SwXTextFieldTypes::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return sal_True; // they always exist
}

void SAL_CALL SwXTextFieldTypes::refresh() throw (uno::RuntimeException, std::exception)
{
    {
        SolarMutexGuard aGuard;
        if (!IsValid())
            throw uno::RuntimeException();
        UnoActionContext aContext(GetDoc());
        GetDoc()->getIDocumentStatistics().UpdateDocStat( false, true );
        GetDoc()->getIDocumentFieldsAccess().UpdateFields(0, false);
    }
    // call refresh listeners (without SolarMutex locked)
    lang::EventObject const event(static_cast< ::cppu::OWeakObject*>(this));
    m_pImpl->m_RefreshListeners.notifyEach(
            & util::XRefreshListener::refreshed, event);
}

void SAL_CALL SwXTextFieldTypes::addRefreshListener(
        const uno::Reference<util::XRefreshListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.addInterface(xListener);
}

void SAL_CALL SwXTextFieldTypes::removeRefreshListener(
        const uno::Reference<util::XRefreshListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.removeInterface(xListener);
}

class SwXFieldEnumeration::Impl
    : public SwClient
{

public:
    SwDoc * m_pDoc;

    uno::Sequence< uno::Reference<text::XTextField> > m_Items;
    sal_Int32       m_nNextIndex;  ///< index of next element to be returned

    explicit Impl(SwDoc & rDoc)
        : SwClient(rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD))
        , m_pDoc(& rDoc)
        , m_nNextIndex(0)
    { }

protected:
    // SwClient
    virtual void Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew) override;
};

OUString SAL_CALL
SwXFieldEnumeration::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("SwXFieldEnumeration");
}

sal_Bool SAL_CALL SwXFieldEnumeration::supportsService(const OUString& rServiceName)
throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXFieldEnumeration::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
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

    const SwFieldTypes* pFieldTypes = m_pImpl->m_pDoc->getIDocumentFieldsAccess().GetFieldTypes();
    const size_t nCount = pFieldTypes->size();
    for(size_t nType = 0;  nType < nCount;  ++nType)
    {
        const SwFieldType *pCurType = (*pFieldTypes)[nType];
        SwIterator<SwFormatField,SwFieldType> aIter( *pCurType );
        const SwFormatField* pCurFieldFormat = aIter.First();
        while (pCurFieldFormat)
        {
            const SwTextField *pTextField = pCurFieldFormat->GetTextField();
            // skip fields that are currently not in the document
            // e.g. fields in undo or redo array
            bool bSkip = !pTextField ||
                         !pTextField->GetpTextNode()->GetNodes().IsDocNodes();
            if (!bSkip)
                pItems[ nFillPos++ ] = SwXTextField::CreateXTextField(
                        m_pImpl->m_pDoc, pCurFieldFormat);
            pCurFieldFormat = aIter.Next();

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
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return m_pImpl->m_nNextIndex < m_pImpl->m_Items.getLength();
}

uno::Any SAL_CALL SwXFieldEnumeration::nextElement()
throw (container::NoSuchElementException, lang::WrappedTargetException,
        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (!(m_pImpl->m_nNextIndex < m_pImpl->m_Items.getLength()))
        throw container::NoSuchElementException(
            "SwXFieldEnumeration::nextElement",
            css::uno::Reference<css::uno::XInterface>());

#if OSL_DEBUG_LEVEL > 1
    uno::Reference< text::XTextField > *pItems = m_pImpl->m_Items.getArray();
    (void)pItems;
#endif
    uno::Reference< text::XTextField >  &rxField =
        m_pImpl->m_Items.getArray()[ m_pImpl->m_nNextIndex++ ];
    uno::Any aRet;
    aRet <<= rxField;
    rxField = 0;  // free memory for item that is not longer used
    return aRet;
}

void SwXFieldEnumeration::Impl::Modify(
        SfxPoolItem const*const pOld, SfxPoolItem const*const pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        m_pDoc = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
