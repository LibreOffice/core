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

#include <sal/config.h>
#include <algorithm>
#include <memory>

#include <unofield.hxx>
#include <unofieldcoll.hxx>
#include <unobookmark.hxx>
#include <swtypes.hxx>
#include <cmdid.h>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unocoll.hxx>
#include <sfx2/linkmgr.hxx>
#include <editsh.hxx>
#include <viewsh.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

//undef to prevent error (from sfx2/docfile.cxx)
#undef SEQUENCE
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <authfld.hxx>
#include <flddat.hxx>
#include <dbfld.hxx>
#include <usrfld.hxx>
#include <docufld.hxx>
#include <expfld.hxx>
#include <chpfld.hxx>
#include <flddropdown.hxx>
#include <poolfmt.hxx>
#include <strings.hrc>
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
#include <svl/itemprop.hxx>
#include <svl/listener.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <textapi.hxx>
#include <fmtmeta.hxx>
#include <vector>

using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

// case-corrected version of the first part for the service names (see #i67811)
constexpr OUStringLiteral COM_TEXT_FLDMASTER_CC = u"com.sun.star.text.fieldmaster.";

// note: this thing is indexed as an array, so do not insert/remove entries!
const sal_uInt16 aDocInfoSubTypeFromService[] =
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
    DI_SUBJECT,                 //PROPERTY_MAP_FLDTYP_DOCINFO_SUBJECT
    DI_TITLE,                   //PROPERTY_MAP_FLDTYP_DOCINFO_TITLE
    DI_DOCNO                    //PROPERTY_MAP_FLDTYP_DOCINFO_REVISION
};

namespace {

struct ServiceIdResId
{
    SwFieldIds    nResId;
    SwServiceType nServiceId;
};

}

const ServiceIdResId aServiceToRes[] =
{
    {SwFieldIds::DateTime,           SwServiceType::FieldTypeDateTime              },
    {SwFieldIds::User,               SwServiceType::FieldTypeUser                  },
    {SwFieldIds::SetExp,             SwServiceType::FieldTypeSetExp                },
    {SwFieldIds::GetExp,             SwServiceType::FieldTypeGetExp                },
    {SwFieldIds::Filename,           SwServiceType::FieldTypeFileName              },
    {SwFieldIds::PageNumber,         SwServiceType::FieldTypePageNum               },
    {SwFieldIds::Author,             SwServiceType::FieldTypeAuthor                },
    {SwFieldIds::Chapter,            SwServiceType::FieldTypeChapter               },
    {SwFieldIds::GetRef,             SwServiceType::FieldTypeGetReference          },
    {SwFieldIds::HiddenText,         SwServiceType::FieldTypeConditionedText       },
    {SwFieldIds::Postit,             SwServiceType::FieldTypeAnnotation            },
    {SwFieldIds::Input,              SwServiceType::FieldTypeInput                 },
    {SwFieldIds::Macro,              SwServiceType::FieldTypeMacro                 },
    {SwFieldIds::Dde,                SwServiceType::FieldTypeDDE                   },
    {SwFieldIds::HiddenPara,         SwServiceType::FieldTypeHiddenPara            },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfo               },
    {SwFieldIds::TemplateName,       SwServiceType::FieldTypeTemplateName          },
    {SwFieldIds::ExtUser,            SwServiceType::FieldTypeUserExt               },
    {SwFieldIds::RefPageSet,         SwServiceType::FieldTypeRefPageSet            },
    {SwFieldIds::RefPageGet,         SwServiceType::FieldTypeRefPageGet            },
    {SwFieldIds::JumpEdit,           SwServiceType::FieldTypeJumpEdit              },
    {SwFieldIds::Script,             SwServiceType::FieldTypeScript                },
    {SwFieldIds::DbNextSet,          SwServiceType::FieldTypeDatabaseNextSet       },
    {SwFieldIds::DbNumSet,           SwServiceType::FieldTypeDatabaseNumSet        },
    {SwFieldIds::DbSetNumber,        SwServiceType::FieldTypeDatabaseSetNum        },
    {SwFieldIds::Database,           SwServiceType::FieldTypeDatabase              },
    {SwFieldIds::DatabaseName,       SwServiceType::FieldTypeDatabaseName          },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypePageCount             },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypeParagraphCount        },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypeWordCount             },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypeCharacterCount        },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypeTableCount            },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypeGraphicObjectCount    },
    {SwFieldIds::DocStat,            SwServiceType::FieldTypeEmbeddedObjectCount   },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoChangeAuthor   },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoChangeDateTime },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoEditTime       },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoDescription    },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoCreateAuthor   },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoCreateDateTime },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoCustom         },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoPrintAuthor    },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoPrintDateTime  },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoKeywords       },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoSubject        },
    {SwFieldIds::DocInfo,            SwServiceType::FieldTypeDocInfoTitle          },
    {SwFieldIds::Input,              SwServiceType::FieldTypeInputUser             },
    {SwFieldIds::HiddenText,         SwServiceType::FieldTypeHiddenText            },
    {SwFieldIds::TableOfAuthorities, SwServiceType::FieldTypeBibliography          },
    {SwFieldIds::CombinedChars,      SwServiceType::FieldTypeCombinedCharacters    },
    {SwFieldIds::Dropdown,           SwServiceType::FieldTypeDropdown              },
    {SwFieldIds::Table,              SwServiceType::FieldTypeTableFormula          }
};

static SwFieldIds lcl_ServiceIdToResId(SwServiceType nServiceId)
{
    for (size_t i=0; i<SAL_N_ELEMENTS(aServiceToRes); ++i)
        if (aServiceToRes[i].nServiceId == nServiceId)
            return aServiceToRes[i].nResId;
#if OSL_DEBUG_LEVEL > 0
    OSL_FAIL("service id not found");
#endif
    return SwFieldIds::Unknown;
}

static SwServiceType lcl_GetServiceForField( const SwField& rField )
{
    const SwFieldIds nWhich = rField.Which();
    SwServiceType nSrvId = SwServiceType::Invalid;
    //special handling for some fields
    switch( nWhich )
    {
    case SwFieldIds::Input:
        if( INP_USR == (rField.GetSubType() & 0x00ff) )
            nSrvId = SwServiceType::FieldTypeInputUser;
        break;

    case SwFieldIds::DocInfo:
        {
            const sal_uInt16 nSubType = rField.GetSubType();
            switch( nSubType & 0xff )
            {
            case DI_CHANGE:
                nSrvId = ((nSubType&0x300) == DI_SUB_AUTHOR)
                        ? SwServiceType::FieldTypeDocInfoChangeAuthor
                        : SwServiceType::FieldTypeDocInfoChangeDateTime;
                break;
            case DI_CREATE:
                nSrvId = ((nSubType&0x300) == DI_SUB_AUTHOR)
                        ? SwServiceType::FieldTypeDocInfoCreateAuthor
                        : SwServiceType::FieldTypeDocInfoCreateDateTime;
                break;
            case DI_PRINT:
                nSrvId = ((nSubType&0x300) == DI_SUB_AUTHOR)
                        ? SwServiceType::FieldTypeDocInfoPrintAuthor
                        : SwServiceType::FieldTypeDocInfoPrintDateTime;
                break;
            case DI_EDIT:   nSrvId = SwServiceType::FieldTypeDocInfoEditTime;break;
            case DI_COMMENT:nSrvId = SwServiceType::FieldTypeDocInfoDescription;break;
            case DI_KEYS:   nSrvId = SwServiceType::FieldTypeDocInfoKeywords;break;
            case DI_SUBJECT:nSrvId = SwServiceType::FieldTypeDocInfoSubject;  break;
            case DI_TITLE:  nSrvId = SwServiceType::FieldTypeDocInfoTitle;    break;
            case DI_DOCNO:  nSrvId = SwServiceType::FieldTypeDocInfoRevision; break;
            case DI_CUSTOM: nSrvId = SwServiceType::FieldTypeDocInfoCustom;   break;
            }
        }
        break;

    case SwFieldIds::HiddenText:
        nSrvId = SwFieldTypesEnum::ConditionalText == static_cast<SwFieldTypesEnum>(rField.GetSubType())
                        ? SwServiceType::FieldTypeConditionedText
                        : SwServiceType::FieldTypeHiddenText;
        break;

    case SwFieldIds::DocStat:
        {
            switch( rField.GetSubType() )
            {
            case DS_PAGE: nSrvId = SwServiceType::FieldTypePageCount; break;
            case DS_PARA: nSrvId = SwServiceType::FieldTypeParagraphCount; break;
            case DS_WORD: nSrvId = SwServiceType::FieldTypeWordCount     ; break;
            case DS_CHAR: nSrvId = SwServiceType::FieldTypeCharacterCount; break;
            case DS_TBL:  nSrvId = SwServiceType::FieldTypeTableCount    ; break;
            case DS_GRF:  nSrvId = SwServiceType::FieldTypeGraphicObjectCount; break;
            case DS_OLE:  nSrvId = SwServiceType::FieldTypeEmbeddedObjectCount; break;
            }
        }
        break;
    default: break;
    }
    if( SwServiceType::Invalid == nSrvId )
    {
        for( const ServiceIdResId* pMap = aServiceToRes;
                SwFieldIds::Unknown != pMap->nResId; ++pMap )
            if( nWhich == pMap->nResId )
            {
                nSrvId = pMap->nServiceId;
                break;
            }
    }
#if OSL_DEBUG_LEVEL > 0
    if( SwServiceType::Invalid == nSrvId )
        OSL_FAIL("resid not found");
#endif
    return nSrvId;
}

static sal_uInt16 lcl_GetPropMapIdForFieldType( SwFieldIds nWhich )
{
    sal_uInt16 nId;
    switch( nWhich )
    {
    case SwFieldIds::User:     nId = PROPERTY_MAP_FLDMSTR_USER;            break;
    case SwFieldIds::Database: nId = PROPERTY_MAP_FLDMSTR_DATABASE;        break;
    case SwFieldIds::SetExp:   nId = PROPERTY_MAP_FLDMSTR_SET_EXP;         break;
    case SwFieldIds::Dde:      nId = PROPERTY_MAP_FLDMSTR_DDE;             break;
    case SwFieldIds::TableOfAuthorities:
                               nId = PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY;    break;
    default:                   nId = PROPERTY_MAP_FLDMSTR_DUMMY0;
    }
    return nId;
}

static sal_Int32 lcl_PropName2TokenPos(std::u16string_view rPropertyName)
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

static sal_uInt16 GetFieldTypeMId( std::u16string_view rProperty, const SwFieldType& rTyp )
{
    sal_uInt16 nId = lcl_GetPropMapIdForFieldType( rTyp.Which() );
    const SfxItemPropertySet* pSet = aSwMapProvider.GetPropertySet( nId );
    if( !pSet )
        nId = USHRT_MAX;
    else
    {
        const SfxItemPropertyMapEntry* pEntry = pSet->getPropertyMap().getByName(rProperty);
        nId = pEntry ? pEntry->nWID : USHRT_MAX;
    }
    return nId;
}

static sal_uInt16 lcl_GetPropertyMapOfService( SwServiceType nServiceId )
{
    sal_uInt16 nRet;
    switch ( nServiceId)
    {
    case SwServiceType::FieldTypeDateTime: nRet = PROPERTY_MAP_FLDTYP_DATETIME; break;
    case SwServiceType::FieldTypeUser: nRet = PROPERTY_MAP_FLDTYP_USER; break;
    case SwServiceType::FieldTypeSetExp: nRet = PROPERTY_MAP_FLDTYP_SET_EXP; break;
    case SwServiceType::FieldTypeGetExp: nRet = PROPERTY_MAP_FLDTYP_GET_EXP; break;
    case SwServiceType::FieldTypeFileName: nRet = PROPERTY_MAP_FLDTYP_FILE_NAME; break;
    case SwServiceType::FieldTypePageNum: nRet = PROPERTY_MAP_FLDTYP_PAGE_NUM; break;
    case SwServiceType::FieldTypeAuthor: nRet = PROPERTY_MAP_FLDTYP_AUTHOR; break;
    case SwServiceType::FieldTypeChapter: nRet = PROPERTY_MAP_FLDTYP_CHAPTER; break;
    case SwServiceType::FieldTypeGetReference: nRet = PROPERTY_MAP_FLDTYP_GET_REFERENCE; break;
    case SwServiceType::FieldTypeConditionedText: nRet = PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT; break;
    case SwServiceType::FieldTypeAnnotation: nRet = PROPERTY_MAP_FLDTYP_ANNOTATION; break;
    case SwServiceType::FieldTypeInputUser:
    case SwServiceType::FieldTypeInput: nRet = PROPERTY_MAP_FLDTYP_INPUT; break;
    case SwServiceType::FieldTypeMacro: nRet = PROPERTY_MAP_FLDTYP_MACRO; break;
    case SwServiceType::FieldTypeDDE: nRet = PROPERTY_MAP_FLDTYP_DDE; break;
    case SwServiceType::FieldTypeHiddenPara: nRet = PROPERTY_MAP_FLDTYP_HIDDEN_PARA; break;
    case SwServiceType::FieldTypeDocInfo: nRet = PROPERTY_MAP_FLDTYP_DOC_INFO; break;
    case SwServiceType::FieldTypeTemplateName: nRet = PROPERTY_MAP_FLDTYP_TEMPLATE_NAME; break;
    case SwServiceType::FieldTypeUserExt: nRet = PROPERTY_MAP_FLDTYP_USER_EXT; break;
    case SwServiceType::FieldTypeRefPageSet: nRet = PROPERTY_MAP_FLDTYP_REF_PAGE_SET; break;
    case SwServiceType::FieldTypeRefPageGet: nRet = PROPERTY_MAP_FLDTYP_REF_PAGE_GET; break;
    case SwServiceType::FieldTypeJumpEdit: nRet = PROPERTY_MAP_FLDTYP_JUMP_EDIT; break;
    case SwServiceType::FieldTypeScript: nRet = PROPERTY_MAP_FLDTYP_SCRIPT; break;
    case SwServiceType::FieldTypeDatabaseNextSet: nRet = PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET; break;
    case SwServiceType::FieldTypeDatabaseNumSet: nRet = PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET; break;
    case SwServiceType::FieldTypeDatabaseSetNum: nRet = PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM; break;
    case SwServiceType::FieldTypeDatabase: nRet = PROPERTY_MAP_FLDTYP_DATABASE; break;
    case SwServiceType::FieldTypeDatabaseName: nRet = PROPERTY_MAP_FLDTYP_DATABASE_NAME; break;
    case SwServiceType::FieldTypeTableFormula: nRet = PROPERTY_MAP_FLDTYP_TABLE_FORMULA; break;
    case SwServiceType::FieldTypePageCount:
    case SwServiceType::FieldTypeParagraphCount:
    case SwServiceType::FieldTypeWordCount:
    case SwServiceType::FieldTypeCharacterCount:
    case SwServiceType::FieldTypeTableCount:
    case SwServiceType::FieldTypeGraphicObjectCount:
    case SwServiceType::FieldTypeEmbeddedObjectCount: nRet = PROPERTY_MAP_FLDTYP_DOCSTAT; break;
    case SwServiceType::FieldTypeDocInfoChangeAuthor:
    case SwServiceType::FieldTypeDocInfoCreateAuthor:
    case SwServiceType::FieldTypeDocInfoPrintAuthor: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR; break;
    case SwServiceType::FieldTypeDocInfoChangeDateTime:
    case SwServiceType::FieldTypeDocInfoCreateDateTime:
    case SwServiceType::FieldTypeDocInfoPrintDateTime: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME; break;
    case SwServiceType::FieldTypeDocInfoEditTime: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME; break;
    case SwServiceType::FieldTypeDocInfoCustom: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM; break;
    case SwServiceType::FieldTypeDocInfoDescription:
    case SwServiceType::FieldTypeDocInfoKeywords:
    case SwServiceType::FieldTypeDocInfoSubject:
    case SwServiceType::FieldTypeDocInfoTitle: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_MISC; break;
    case SwServiceType::FieldTypeDocInfoRevision: nRet = PROPERTY_MAP_FLDTYP_DOCINFO_REVISION; break;
    case SwServiceType::FieldTypeBibliography: nRet = PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY; break;
    case SwServiceType::FieldTypeDummy0:
    case SwServiceType::FieldTypeCombinedCharacters: nRet = PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS; break;
    case SwServiceType::FieldTypeDropdown: nRet = PROPERTY_MAP_FLDTYP_DROPDOWN; break;
    case SwServiceType::FieldTypeDummy4:
    case SwServiceType::FieldTypeDummy5:
    case SwServiceType::FieldTypeDummy6:
    case SwServiceType::FieldTypeDummy7:
                nRet = PROPERTY_MAP_FLDTYP_DUMMY_0; break;
    case SwServiceType::FieldMasterUser: nRet = PROPERTY_MAP_FLDMSTR_USER; break;
    case SwServiceType::FieldMasterDDE: nRet = PROPERTY_MAP_FLDMSTR_DDE; break;
    case SwServiceType::FieldMasterSetExp: nRet = PROPERTY_MAP_FLDMSTR_SET_EXP; break;
    case SwServiceType::FieldMasterDatabase: nRet = PROPERTY_MAP_FLDMSTR_DATABASE; break;
    case SwServiceType::FieldMasterBibliography: nRet = PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY; break;
    case SwServiceType::FieldMasterDummy2:
    case SwServiceType::FieldMasterDummy3:
    case SwServiceType::FieldMasterDummy4:
    case SwServiceType::FieldMasterDummy5: nRet = PROPERTY_MAP_FLDMSTR_DUMMY0; break;
    case SwServiceType::FieldTypeHiddenText: nRet = PROPERTY_MAP_FLDTYP_HIDDEN_TEXT; break;
    default:
        nRet = USHRT_MAX;
    }
    assert(nRet != USHRT_MAX && "wrong service id");
    return nRet;
}

class SwXFieldMaster::Impl
    : public SvtListener
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper3

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> m_EventListeners;

    SwDoc*          m_pDoc;
    SwFieldType* m_pType;

    SwFieldIds      m_nResTypeId;

    OUString        m_sParam1;  // Content / Database / NumberingSeparator
    OUString        m_sParam2;  // -    /DataTablename
    OUString        m_sParam3;  // -    /DataFieldName
    OUString        m_sParam5;  // -    /DataBaseURL
    double          m_fParam1;  // Value / -
    sal_Int8        m_nParam1;  // ChapterNumberingLevel
    bool            m_bParam1;  // IsExpression
    sal_Int32       m_nParam2;

    Impl(SwPageDesc* const pPageDesc, SwDoc* pDoc, SwFieldIds nResId)
        : m_EventListeners(m_Mutex)
        , m_pDoc(pDoc)
        , m_pType(nullptr)
        , m_nResTypeId(nResId)
        , m_fParam1(0.0)
        , m_nParam1(-1)
        , m_bParam1(false)
        , m_nParam2(0)
    {
        StartListening(pPageDesc->GetNotifier());
    }

    Impl(SwFieldType* const pType, SwDoc* pDoc, SwFieldIds nResId)
        : m_EventListeners(m_Mutex)
        , m_pDoc(pDoc)
        , m_pType(pType)
        , m_nResTypeId(nResId)
        , m_fParam1(0.0)
        , m_nParam1(-1)
        , m_bParam1(false)
        , m_nParam2(0)
    {
        StartListening(m_pType->GetNotifier());
    }
    void SetFieldType(SwFieldType* pType)
    {
        EndListeningAll();
        m_pType = pType;
        StartListening(m_pType->GetNotifier());
    }
protected:
    virtual void Notify(const SfxHint& rHint) override;
};

const uno::Sequence< sal_Int8 > & SwXFieldMaster::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXFieldMasterUnoTunnelId;
    return theSwXFieldMasterUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL
SwXFieldMaster::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return comphelper::getSomethingImpl<SwXFieldMaster>(rId, this);
}

OUString SAL_CALL
SwXFieldMaster::getImplementationName()
{
    return "SwXFieldMaster";
}

namespace
{

OUString getServiceName(const SwFieldIds aId)
{
    const char* pEntry;
    switch (aId)
    {
        case SwFieldIds::User:
            pEntry = "User";
            break;
        case SwFieldIds::Database:
            pEntry = "Database";
            break;
        case SwFieldIds::SetExp:
            pEntry = "SetExpression";
            break;
        case SwFieldIds::Dde:
            pEntry = "DDE";
            break;
        case SwFieldIds::TableOfAuthorities:
            pEntry = "Bibliography";
            break;
        default:
            return OUString();
    }

    return "com.sun.star.text.fieldmaster." + OUString::createFromAscii(pEntry);
}

}

sal_Bool SAL_CALL SwXFieldMaster::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
SwXFieldMaster::getSupportedServiceNames()
{
    return { "com.sun.star.text.TextFieldMaster", getServiceName(m_pImpl->m_nResTypeId) };
}

SwXFieldMaster::SwXFieldMaster(SwDoc *const pDoc, SwFieldIds const nResId)
    : m_pImpl(new Impl(pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD), pDoc, nResId))
{
}

SwXFieldMaster::SwXFieldMaster(SwFieldType& rType, SwDoc * pDoc)
    : m_pImpl(new Impl(&rType, pDoc, rType.Which()))
{
}

SwXFieldMaster::~SwXFieldMaster()
{
}

uno::Reference<beans::XPropertySet>
SwXFieldMaster::CreateXFieldMaster(SwDoc * pDoc, SwFieldType *const pType,
        SwFieldIds nResId)
{
    // re-use existing SwXFieldMaster
    uno::Reference<beans::XPropertySet> xFM;
    if (pType)
    {
        xFM = pType->GetXObject();
    }
    if (!xFM.is())
    {
        SwXFieldMaster *const pFM( pType
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
{
    SolarMutexGuard aGuard;
    uno::Reference< beans::XPropertySetInfo >  aRef =
                        aSwMapProvider.GetPropertySet(
            lcl_GetPropMapIdForFieldType(m_pImpl->m_nResTypeId))->getPropertySetInfo();
    return aRef;
}

void SAL_CALL SwXFieldMaster::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    SolarMutexGuard aGuard;
    SwFieldType* pType = GetFieldType(true);
    if(pType)
    {
        bool bSetValue = true;
        if( rPropertyName == UNO_NAME_SUB_TYPE )
        {
            const std::vector<OUString>& rExtraArr(
                    SwStyleNameMapper::GetExtraUINameArray());
            const OUString sTypeName = pType->GetName();
            static sal_uInt16 nIds[] =
            {
                RES_POOLCOLL_LABEL_DRAWING - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_ABB - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_TABLE - RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_FRAME- RES_POOLCOLL_EXTRA_BEGIN,
                RES_POOLCOLL_LABEL_FIGURE - RES_POOLCOLL_EXTRA_BEGIN,
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
            if ( USHRT_MAX == nMemberValueId )
            {
                throw beans::UnknownPropertyException(
                    "Unknown property: " + rPropertyName,
                    static_cast< cppu::OWeakObject * >( this ) );
            }

            pType->PutValue( rValue, nMemberValueId );
            if ( pType->Which() == SwFieldIds::User )
            {
                // trigger update of User field in order to get depending Input Fields updated.
                pType->UpdateFields();
            }

        }
    }
    else if (m_pImpl->m_pDoc && rPropertyName == UNO_NAME_NAME)
    {
        OUString sTypeName;
        rValue >>= sTypeName;
        SwFieldType * pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().GetFieldType(
                m_pImpl->m_nResTypeId, sTypeName, false);

        if(pType2 ||
            (SwFieldIds::SetExp == m_pImpl->m_nResTypeId &&
            ( sTypeName == SwResId(STR_POOLCOLL_LABEL_TABLE) ||
              sTypeName == SwResId(STR_POOLCOLL_LABEL_DRAWING) ||
              sTypeName == SwResId(STR_POOLCOLL_LABEL_FRAME) ||
              sTypeName == SwResId(STR_POOLCOLL_LABEL_ABB) ||
              sTypeName == SwResId(STR_POOLCOLL_LABEL_FIGURE) )))
        {
            throw lang::IllegalArgumentException();
        }

        switch (m_pImpl->m_nResTypeId)
        {
            case SwFieldIds::User :
            {
                SwUserFieldType aType(m_pImpl->m_pDoc, sTypeName);
                pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
                static_cast<SwUserFieldType*>(pType2)->SetContent(m_pImpl->m_sParam1);
                static_cast<SwUserFieldType*>(pType2)->SetValue(m_pImpl->m_fParam1);
                static_cast<SwUserFieldType*>(pType2)->SetType(m_pImpl->m_bParam1
                    ? nsSwGetSetExpType::GSE_EXPR : nsSwGetSetExpType::GSE_STRING);
            }
            break;
            case SwFieldIds::Dde :
            {
                SwDDEFieldType aType(sTypeName, m_pImpl->m_sParam1,
                    m_pImpl->m_bParam1 ? SfxLinkUpdateMode::ALWAYS : SfxLinkUpdateMode::ONCALL);
                pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
            }
            break;
            case SwFieldIds::SetExp :
            {
                SwSetExpFieldType aType(m_pImpl->m_pDoc, sTypeName);
                if (!m_pImpl->m_sParam1.isEmpty())
                    aType.SetDelimiter(OUString(m_pImpl->m_sParam1[0]));
                if (m_pImpl->m_nParam1 > -1 && m_pImpl->m_nParam1 < MAXLEVEL)
                    aType.SetOutlineLvl(m_pImpl->m_nParam1);
                pType2 = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
            }
            break;
            case SwFieldIds::Database :
            {
                rValue >>= m_pImpl->m_sParam3;
                pType2 = GetFieldType();
            }
            break;
            default: break;
        }
        if (!pType2)
        {
            throw uno::RuntimeException("no field type found!", *this);
        }
        m_pImpl->SetFieldType(pType2);
    }
    else
    {
        switch (m_pImpl->m_nResTypeId)
        {
        case SwFieldIds::User:
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
        case SwFieldIds::Database:
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
        case  SwFieldIds::SetExp:
            if(rPropertyName == UNO_NAME_NUMBERING_SEPARATOR)
                rValue >>= m_pImpl->m_sParam1;
            else if(rPropertyName == UNO_NAME_CHAPTER_NUMBERING_LEVEL)
                rValue >>= m_pImpl->m_nParam1;
            break;
        case SwFieldIds::Dde:
            {
                sal_Int32 nPart = lcl_PropName2TokenPos(rPropertyName);
                if(nPart  < 3 )
                {
                    if (m_pImpl->m_sParam1.isEmpty())
                    {
                        m_pImpl->m_sParam1
                            = OUStringChar(sfx2::cTokenSeparator)
                            + OUStringChar(sfx2::cTokenSeparator);
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
    if (!bDontCreate && SwFieldIds::Database == m_pImpl->m_nResTypeId
        && !m_pImpl->m_pType && m_pImpl->m_pDoc)
    {
        SwDBData aData;

        // set DataSource
        svx::ODataAccessDescriptor aAcc;
        if (!m_pImpl->m_sParam1.isEmpty())
            aAcc[svx::DataAccessDescriptorProperty::DataSource]        <<= m_pImpl->m_sParam1; // DataBaseName
        else if (!m_pImpl->m_sParam5.isEmpty())
            aAcc[svx::DataAccessDescriptorProperty::DatabaseLocation]  <<= m_pImpl->m_sParam5; // DataBaseURL
        aData.sDataSource = aAcc.getDataSource();

        aData.sCommand = m_pImpl->m_sParam2;
        aData.nCommandType = m_pImpl->m_nParam2;

        SwDBFieldType aType(m_pImpl->m_pDoc, m_pImpl->m_sParam3, aData);
        SwFieldType *const pType = m_pImpl->m_pDoc->getIDocumentFieldsAccess().InsertFieldType(aType);
        m_pImpl->SetFieldType(pType);
    }
    return m_pImpl->m_pType;
}

uno::Any SAL_CALL
SwXFieldMaster::getPropertyValue(const OUString& rPropertyName)
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
            std::vector<SwFormatField*> vpFields;
            pType->GatherFields(vpFields);
            uno::Sequence<uno::Reference <text::XDependentTextField> > aSeq(vpFields.size());
            std::transform(vpFields.begin(), vpFields.end(), aSeq.getArray(),
                    [this](SwFormatField* pF) { return uno::Reference<text::XDependentTextField>(SwXTextField::CreateXTextField(m_pImpl->m_pDoc, pF), uno::UNO_QUERY); });
            aRet <<= aSeq;
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

                OUString *pStr = nullptr;     // only one of this properties will return
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
            case SwFieldIds::User:
                if( rPropertyName == UNO_NAME_CONTENT )
                    aRet <<= m_pImpl->m_sParam1;
                else if(rPropertyName == UNO_NAME_VALUE)
                    aRet <<= m_pImpl->m_fParam1;
                else if(rPropertyName == UNO_NAME_IS_EXPRESSION)
                    aRet <<= m_pImpl->m_bParam1;
                break;
            case SwFieldIds::Database:
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
            case SwFieldIds::SetExp:
                if(rPropertyName == UNO_NAME_NUMBERING_SEPARATOR)
                    aRet <<= m_pImpl->m_sParam1;
                else if(rPropertyName == UNO_NAME_CHAPTER_NUMBERING_LEVEL)
                    aRet <<= m_pImpl->m_nParam1;
                break;
            case SwFieldIds::Dde:
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

void SwXFieldMaster::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFieldMaster::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXFieldMaster::dispose()
{
    SolarMutexGuard aGuard;
    SwFieldType *const pFieldType = GetFieldType(true);
    if (!pFieldType)
        throw uno::RuntimeException();
    size_t nTypeIdx = SIZE_MAX;
    const SwFieldTypes* pTypes = m_pImpl->m_pDoc->getIDocumentFieldsAccess().GetFieldTypes();
    for( size_t i = 0; i < pTypes->size(); i++ )
    {
        if((*pTypes)[i].get()== pFieldType)
            nTypeIdx = i;
    }

    // first delete all fields
    std::vector<SwFormatField*> vpFields;
    pFieldType->GatherFields(vpFields);
    for(auto pField : vpFields)
        SwTextField::DeleteTextField(*pField->GetTextField());
    // then delete FieldType
    m_pImpl->m_pDoc->getIDocumentFieldsAccess().RemoveFieldType(nTypeIdx);
}

void SAL_CALL SwXFieldMaster::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXFieldMaster::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

void SwXFieldMaster::Impl::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_pDoc = nullptr;
        m_pType = nullptr;
        uno::Reference<uno::XInterface> const xThis(m_wThis);
        if (!xThis.is())
        {   // fdo#72695: if UNO object is already dead, don't revive it with event
            return;
        }
        lang::EventObject const ev(xThis);
        m_EventListeners.disposeAndClear(ev);
    }
}

OUString SwXFieldMaster::GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc)
{
    const OUString sName(rType.GetName());
    if(SwFieldIds::SetExp == rType.Which())
    {
        const SwFieldTypes* pTypes = rDoc.getIDocumentFieldsAccess().GetFieldTypes();
        for( size_t i = 0; i <= o3tl::make_unsigned(INIT_FLDTYPES); i++ )
        {
            if((*pTypes)[i].get() == &rType)
            {
                return SwStyleNameMapper::GetProgName( sName, SwGetPoolIdFromName::TxtColl );
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
        SwStyleNameMapper::GetProgName(sTypeName, SwGetPoolIdFromName::TxtColl ));
    if(sProgName != sTypeName)
    {
        const OUString sSource = bQuery ? sTypeName : sProgName;
        const OUString sDest = bQuery ? sProgName : sTypeName;
        if(rFormula.startsWith(sSource))
        {
            return sDest + rFormula.subView(sSource.getLength());
        }
    }
    return rFormula;
}

namespace {

struct SwFieldProperties_Impl
{
    OUString    sPar1;
    OUString    sPar2;
    OUString    sPar3;
    OUString    sPar4;
    Date            aDate;
    double          fDouble;
    uno::Sequence<beans::PropertyValue> aPropSeq;
    uno::Sequence<OUString> aStrings;
    std::unique_ptr<util::DateTime> pDateTime;

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
};

}

class SwXTextField::Impl
    : public SvtListener
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper3
    SwFieldType* m_pFieldType;
    SwFormatField* m_pFormatField;

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> m_EventListeners;

    SwDoc* m_pDoc;
    rtl::Reference<SwTextAPIObject> m_xTextObject;
    bool m_bIsDescriptor;
    bool m_bCallUpdate;
    SwServiceType m_nServiceId;
    OUString m_sTypeName;
    std::unique_ptr<SwFieldProperties_Impl> m_pProps;

    Impl(SwDoc *const pDoc, SwFormatField *const pFormat, SwServiceType nServiceId)
        : m_pFieldType(nullptr)
        , m_pFormatField(pFormat)
        , m_EventListeners(m_Mutex)
        , m_pDoc(pDoc)
        , m_bIsDescriptor(pFormat == nullptr)
        , m_bCallUpdate(false)
        , m_nServiceId(pFormat
                ? lcl_GetServiceForField(*pFormat->GetField())
                : nServiceId)
        , m_pProps(pFormat ? nullptr : new SwFieldProperties_Impl)
    {
        if(m_pFormatField)
            StartListening(m_pFormatField->GetNotifier());
    }

    virtual ~Impl() override
    {
        if (m_xTextObject.is())
        {
            m_xTextObject->DisposeEditSource();
        }
    }

    void SetFormatField(SwFormatField* pFormatField, SwDoc* pDoc)
    {
        m_pFormatField = pFormatField;
        m_pDoc = pDoc;
        if(m_pFormatField)
        {
            EndListeningAll();
            StartListening(m_pFormatField->GetNotifier());
        }
    }
    SwFormatField* GetFormatField()
    {
        return m_pFormatField;
    }
    bool IsDescriptor() const
    {
        // ideally should be: !m_pFormatField && m_pDoc
        // but: SwXServiceProvider::MakeInstance() passes nullptr SwDoc, see comment there
        return m_bIsDescriptor;
    }
    void Invalidate();

    const SwField* GetField() const;

    SwFieldType* GetFieldType() const
    {
        if(!m_pDoc && !IsDescriptor())
            throw uno::RuntimeException();
        else if (IsDescriptor())
            return m_pFieldType;

        return m_pFormatField->GetField()->GetTyp();
    }
    void SetFieldType(SwFieldType& rType)
    {
        EndListeningAll();
        m_pFieldType = &rType;
        StartListening(m_pFieldType->GetNotifier());
    }
    void ClearFieldType()
    {
        SvtListener::EndListeningAll();
        m_pFieldType = nullptr;
    }
    virtual void Notify(const SfxHint&) override;
};

const uno::Sequence< sal_Int8 > & SwXTextField::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXTextFieldUnoTunnelId;
    return theSwXTextFieldUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL
SwXTextField::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return comphelper::getSomethingImpl<SwXTextField>(rId, this);
}

SwXTextField::SwXTextField(
    SwServiceType nServiceId,
    SwDoc* pDoc)
    : m_pImpl(new Impl(pDoc, nullptr, nServiceId))
{
    //Set visible as default!
    if ( SwServiceType::FieldTypeSetExp == nServiceId
         || SwServiceType::FieldTypeDatabaseSetNum == nServiceId
         || SwServiceType::FieldTypeDatabase == nServiceId
         || SwServiceType::FieldTypeDatabaseName == nServiceId )
    {
        m_pImpl->m_pProps->bBool2 = true;
    }
    else if(SwServiceType::FieldTypeTableFormula == nServiceId)
    {
        m_pImpl->m_pProps->bBool1 = true;
    }
    if(SwServiceType::FieldTypeSetExp == nServiceId)
    {
        m_pImpl->m_pProps->nUSHORT2 = USHRT_MAX;
    }
}

SwXTextField::SwXTextField(SwFormatField& rFormat, SwDoc & rDoc)
    : m_pImpl(new Impl(&rDoc, &rFormat, SwServiceType::Invalid))
{
}

SwXTextField::~SwXTextField()
{
}

uno::Reference<text::XTextField>
SwXTextField::CreateXTextField(SwDoc *const pDoc, SwFormatField const* pFormat,
        SwServiceType nServiceId)
{
    assert(!pFormat || pDoc);
    assert(pFormat || nServiceId != SwServiceType::Invalid);
    // re-use existing SwXTextField
    uno::Reference<text::XTextField> xField;
    if (pFormat)
    {
        xField = pFormat->GetXTextField();
    }
    if (!xField.is())
    {
        SwXTextField *const pField( pFormat
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

SwServiceType SwXTextField::GetServiceId() const
{
    return m_pImpl->m_nServiceId;
}

/** Convert between SwSetExpField with InputFlag false and InputFlag true.
    Unfortunately the InputFlag is exposed in the API as "Input" property
    and is mutable; in the UI and in ODF these are 2 different types of
    fields, so the API design is very questionable.
    In order to keep the mutable property, the whole thing has to be
    reconstructed from scratch, to replace the SwTextField hint with
    SwTextInputField or vice versa.
    The SwFormatField will be replaced - it must be, because the Which
    changes - but the SwXTextField *must not* be disposed in the operation,
    it has to be disconnected first and at the end connected to the
    new instance!
 */
void SwXTextField::TransmuteLeadToInputField(SwSetExpField & rField)
{
    assert(rField.GetFormatField()->Which() == (rField.GetInputFlag() ? RES_TXTATR_INPUTFIELD : RES_TXTATR_FIELD));
    uno::Reference<text::XTextField> const xField(
        rField.GetFormatField()->GetXTextField());
    SwXTextField *const pXField = xField.is()
        ? comphelper::getFromUnoTunnel<SwXTextField>(uno::Reference<lang::XUnoTunnel>(xField, uno::UNO_QUERY_THROW))
        : nullptr;
    if (pXField)
        pXField->m_pImpl->SetFormatField(nullptr, nullptr);
    SwTextField *const pOldAttr(rField.GetFormatField()->GetTextField());
    SwSetExpField tempField(rField);
    tempField.SetInputFlag(!rField.GetInputFlag());
    SwFormatField tempFormat(tempField);
    assert(tempFormat.GetField() != &rField);
    assert(tempFormat.GetField() != &tempField); // this copies it again?
    assert(tempFormat.Which() == (static_cast<SwSetExpField const*>(tempFormat.GetField())->GetInputFlag() ? RES_TXTATR_INPUTFIELD : RES_TXTATR_FIELD));
    SwTextNode & rNode(pOldAttr->GetTextNode());
    std::shared_ptr<SwPaM> pPamForTextField;
    IDocumentContentOperations & rIDCO(rNode.GetDoc().getIDocumentContentOperations());
    SwTextField::GetPamForTextField(*pOldAttr, pPamForTextField);
    assert(pPamForTextField);
    sal_Int32 const nStart(pPamForTextField->Start()->nContent.GetIndex());
    rIDCO.DeleteAndJoin(*pPamForTextField);
    // ATTENTION: rField is dead now! hope nobody accesses it...
    bool bSuccess = rIDCO.InsertPoolItem(*pPamForTextField, tempFormat);
    assert(bSuccess);
    (void) bSuccess;
    SwTextField const* pNewAttr(rNode.GetFieldTextAttrAt(nStart, true));
    assert(pNewAttr);
    SwFormatField const& rNewFormat(pNewAttr->GetFormatField());
    assert(rNewFormat.Which() == (static_cast<SwSetExpField const*>(rNewFormat.GetField())->GetInputFlag() ? RES_TXTATR_INPUTFIELD : RES_TXTATR_FIELD));
    assert(static_cast<SwSetExpField const*>(rNewFormat.GetField())->GetInputFlag() == (dynamic_cast<SwTextInputField const*>(pNewAttr) != nullptr));
    if (pXField)
    {
        pXField->m_pImpl->SetFormatField(const_cast<SwFormatField*>(&rNewFormat), &rNode.GetDoc());
        const_cast<SwFormatField&>(rNewFormat).SetXTextField(xField);
    }
}

void SAL_CALL SwXTextField::attachTextFieldMaster(
        const uno::Reference< beans::XPropertySet > & xFieldMaster)
{
    SolarMutexGuard aGuard;

    if (!m_pImpl->IsDescriptor())
        throw uno::RuntimeException();
    uno::Reference< lang::XUnoTunnel > xMasterTunnel(xFieldMaster, uno::UNO_QUERY);
    if (!xMasterTunnel.is())
        throw lang::IllegalArgumentException();
    SwXFieldMaster* pMaster = comphelper::getFromUnoTunnel<SwXFieldMaster>(xMasterTunnel);

    SwFieldType* pFieldType = pMaster ? pMaster->GetFieldType() : nullptr;
    if (!pFieldType ||
        pFieldType->Which() != lcl_ServiceIdToResId(m_pImpl->m_nServiceId))
    {
        throw lang::IllegalArgumentException();
    }
    m_pImpl->m_sTypeName = pFieldType->GetName();
    m_pImpl->SetFieldType(*pFieldType);
}

uno::Reference< beans::XPropertySet > SAL_CALL
SwXTextField::getTextFieldMaster()
{
    SolarMutexGuard aGuard;

    SwFieldType* pType = m_pImpl->GetFieldType();
    uno::Reference<beans::XPropertySet> const xRet(
            SwXFieldMaster::CreateXFieldMaster(m_pImpl->m_pDoc, pType));
    return xRet;
}

OUString SAL_CALL SwXTextField::getPresentation(sal_Bool bShowCommand)
{
    SolarMutexGuard aGuard;

    SwField const*const pField = m_pImpl->GetField();
    if (!pField)
    {
        throw uno::RuntimeException();
    }
    return bShowCommand ? pField->GetFieldName() : pField->ExpandField(true, nullptr);
}

void SAL_CALL SwXTextField::attach(
        const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;
    if (m_pImpl->IsDescriptor())
    {
        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
        SwXTextRange* pRange = comphelper::getFromUnoTunnel<SwXTextRange>(xRangeTunnel);
        OTextCursorHelper* pCursor = comphelper::getFromUnoTunnel<OTextCursorHelper>(xRangeTunnel);

        SwDoc* pDoc = pRange ? &pRange->GetDoc() : pCursor ? pCursor->GetDoc() : nullptr;
        // if a FieldMaster was attached, then the document is already fixed!
        // NOTE: sw.SwXAutoTextEntry unoapi test depends on m_pDoc = 0 being valid
        if (!pDoc || (m_pImpl->m_pDoc && m_pImpl->m_pDoc != pDoc))
            throw lang::IllegalArgumentException();

        SwUnoInternalPaM aPam(*pDoc);
        // this now needs to return TRUE
        ::sw::XTextRangeToSwPaM(aPam, xTextRange);
        std::unique_ptr<SwField> xField;
        switch (m_pImpl->m_nServiceId)
        {
            case SwServiceType::FieldTypeAnnotation:
                {
                    SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Postit);

                    DateTime aDateTime( DateTime::EMPTY );
                    if (m_pImpl->m_pProps->pDateTime)
                    {
                        aDateTime = *(m_pImpl->m_pProps->pDateTime);
                    }
                    SwPostItField* pPostItField = new SwPostItField(
                        static_cast<SwPostItFieldType*>(pFieldType),
                        m_pImpl->m_pProps->sPar1, // author
                        m_pImpl->m_pProps->sPar2, // content
                        m_pImpl->m_pProps->sPar3, // author's initials
                        m_pImpl->m_pProps->sPar4, // name
                        aDateTime,
                        m_pImpl->m_pProps->bBool1 // resolvedflag
                    );
                    if ( m_pImpl->m_xTextObject.is() )
                    {
                        pPostItField->SetTextObject( m_pImpl->m_xTextObject->CreateText() );
                        pPostItField->SetPar2(m_pImpl->m_xTextObject->GetText());
                    }
                    xField.reset(pPostItField);
                }
                break;
            case SwServiceType::FieldTypeScript:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Script);
                xField.reset(new SwScriptField(static_cast<SwScriptFieldType*>(pFieldType),
                        m_pImpl->m_pProps->sPar1, m_pImpl->m_pProps->sPar2,
                        m_pImpl->m_pProps->bBool1));
            }
            break;
            case SwServiceType::FieldTypeDateTime:
            {
                sal_uInt16 nSub = 0;
                if (m_pImpl->m_pProps->bBool1)
                    nSub |= FIXEDFLD;
                if (m_pImpl->m_pProps->bBool2)
                    nSub |= DATEFLD;
                else
                    nSub |= TIMEFLD;
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DateTime);
                SwDateTimeField *const pDTField = new SwDateTimeField(
                        static_cast<SwDateTimeFieldType*>(pFieldType),
                            nSub, m_pImpl->m_pProps->nFormat);
                xField.reset(pDTField);
                if (m_pImpl->m_pProps->fDouble > 0.)
                {
                    pDTField->SetValue(m_pImpl->m_pProps->fDouble);
                }
                if (m_pImpl->m_pProps->pDateTime)
                {
                    uno::Any aVal; aVal <<= *m_pImpl->m_pProps->pDateTime;
                    xField->PutValue( aVal, FIELD_PROP_DATE_TIME );
                }
                pDTField->SetOffset(m_pImpl->m_pProps->nSubType);
            }
            break;
            case SwServiceType::FieldTypeFileName:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Filename);
                sal_Int32 nFormat = m_pImpl->m_pProps->nFormat;
                if (m_pImpl->m_pProps->bBool2)
                    nFormat |= FF_FIXED;
                SwFileNameField *const pFNField = new SwFileNameField(
                        static_cast<SwFileNameFieldType*>(pFieldType), nFormat);
                xField.reset(pFNField);
                if (!m_pImpl->m_pProps->sPar3.isEmpty())
                    pFNField->SetExpansion(m_pImpl->m_pProps->sPar3);
                uno::Any aFormat;
                aFormat <<= m_pImpl->m_pProps->nFormat;
                xField->PutValue( aFormat, FIELD_PROP_FORMAT );
            }
            break;
            case SwServiceType::FieldTypeTemplateName:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::TemplateName);
                xField.reset(new SwTemplNameField(static_cast<SwTemplNameFieldType*>(pFieldType),
                                            m_pImpl->m_pProps->nFormat));
                uno::Any aFormat;
                aFormat <<= m_pImpl->m_pProps->nFormat;
                xField->PutValue(aFormat, FIELD_PROP_FORMAT);
            }
            break;
            case SwServiceType::FieldTypeChapter:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Chapter);
                SwChapterField *const pChapterField = new SwChapterField(
                        static_cast<SwChapterFieldType*>(pFieldType),
                        m_pImpl->m_pProps->nUSHORT1);
                xField.reset(pChapterField);
                pChapterField->SetLevel(m_pImpl->m_pProps->nByte1);
                uno::Any aVal;
                aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
                xField->PutValue(aVal, FIELD_PROP_USHORT1 );
            }
            break;
            case SwServiceType::FieldTypeAuthor:
            {
                tools::Long nFormat = m_pImpl->m_pProps->bBool1 ? AF_NAME : AF_SHORTCUT;
                if (m_pImpl->m_pProps->bBool2)
                    nFormat |= AF_FIXED;

                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Author);
                SwAuthorField *const pAuthorField = new SwAuthorField(
                        static_cast<SwAuthorFieldType*>(pFieldType), nFormat);
                xField.reset(pAuthorField);
                pAuthorField->SetExpansion(m_pImpl->m_pProps->sPar1);
            }
            break;
            case SwServiceType::FieldTypeConditionedText:
            case SwServiceType::FieldTypeHiddenText:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::HiddenText);
                SwHiddenTextField *const pHTField = new SwHiddenTextField(
                        static_cast<SwHiddenTextFieldType*>(pFieldType),
                        m_pImpl->m_pProps->sPar1,
                        m_pImpl->m_pProps->sPar2, m_pImpl->m_pProps->sPar3,
                        SwServiceType::FieldTypeHiddenText == m_pImpl->m_nServiceId ?
                             SwFieldTypesEnum::HiddenText : SwFieldTypesEnum::ConditionalText);
                xField.reset(pHTField);
                pHTField->SetValue(m_pImpl->m_pProps->bBool1);
                uno::Any aVal;
                aVal <<= m_pImpl->m_pProps->sPar4;
                xField->PutValue(aVal, FIELD_PROP_PAR4 );
            }
            break;
            case SwServiceType::FieldTypeHiddenPara:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::HiddenPara);
                SwHiddenParaField *const pHPField = new SwHiddenParaField(
                        static_cast<SwHiddenParaFieldType*>(pFieldType),
                        m_pImpl->m_pProps->sPar1);
                xField.reset(pHPField);
                pHPField->SetHidden(m_pImpl->m_pProps->bBool1);
            }
            break;
            case SwServiceType::FieldTypeGetReference:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::GetRef);
                xField.reset(new SwGetRefField(static_cast<SwGetRefFieldType*>(pFieldType),
                            m_pImpl->m_pProps->sPar1,
                            m_pImpl->m_pProps->sPar4,
                            0,
                            0,
                            0));
                if (!m_pImpl->m_pProps->sPar3.isEmpty())
                    static_cast<SwGetRefField*>(xField.get())->SetExpand(m_pImpl->m_pProps->sPar3);
                uno::Any aVal;
                aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT1);
                xField->PutValue(aVal, FIELD_PROP_USHORT1 );
                aVal <<= static_cast<sal_Int16>(m_pImpl->m_pProps->nUSHORT2);
                xField->PutValue(aVal, FIELD_PROP_USHORT2 );
                aVal <<= m_pImpl->m_pProps->nSHORT1;
                xField->PutValue(aVal, FIELD_PROP_SHORT1 );
            }
            break;
            case SwServiceType::FieldTypeJumpEdit:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::JumpEdit);
                xField.reset(new SwJumpEditField(static_cast<SwJumpEditFieldType*>(pFieldType),
                        m_pImpl->m_pProps->nUSHORT1, m_pImpl->m_pProps->sPar2,
                        m_pImpl->m_pProps->sPar1));
            }
            break;
            case SwServiceType::FieldTypeDocInfoChangeAuthor:
            case SwServiceType::FieldTypeDocInfoChangeDateTime:
            case SwServiceType::FieldTypeDocInfoEditTime:
            case SwServiceType::FieldTypeDocInfoDescription:
            case SwServiceType::FieldTypeDocInfoCreateAuthor:
            case SwServiceType::FieldTypeDocInfoCreateDateTime:
            case SwServiceType::FieldTypeDocInfoCustom:
            case SwServiceType::FieldTypeDocInfoPrintAuthor:
            case SwServiceType::FieldTypeDocInfoPrintDateTime:
            case SwServiceType::FieldTypeDocInfoKeywords:
            case SwServiceType::FieldTypeDocInfoSubject:
            case SwServiceType::FieldTypeDocInfoTitle:
            case SwServiceType::FieldTypeDocInfoRevision:
            case SwServiceType::FieldTypeDocInfo:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DocInfo);
                sal_uInt16 nSubType = aDocInfoSubTypeFromService[
                        static_cast<sal_uInt16>(m_pImpl->m_nServiceId) - sal_uInt16(SwServiceType::FieldTypeDocInfoChangeAuthor)];
                if (SwServiceType::FieldTypeDocInfoChangeDateTime == m_pImpl->m_nServiceId ||
                    SwServiceType::FieldTypeDocInfoCreateDateTime == m_pImpl->m_nServiceId ||
                    SwServiceType::FieldTypeDocInfoPrintDateTime == m_pImpl->m_nServiceId ||
                    SwServiceType::FieldTypeDocInfoEditTime == m_pImpl->m_nServiceId)
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
                xField.reset(new SwDocInfoField(
                        static_cast<SwDocInfoFieldType*>(pFieldType), nSubType,
                        m_pImpl->m_pProps->sPar4, m_pImpl->m_pProps->nFormat));
                if (!m_pImpl->m_pProps->sPar3.isEmpty())
                    static_cast<SwDocInfoField*>(xField.get())->SetExpansion(m_pImpl->m_pProps->sPar3);
            }
            break;
            case SwServiceType::FieldTypeUserExt:
            {
                sal_Int32 nFormat = 0;
                if (m_pImpl->m_pProps->bBool1)
                    nFormat = AF_FIXED;

                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::ExtUser);
                SwExtUserField *const pEUField = new SwExtUserField(
                    static_cast<SwExtUserFieldType*>(pFieldType),
                    m_pImpl->m_pProps->nUSHORT1, nFormat);
                xField.reset(pEUField);
                pEUField->SetExpansion(m_pImpl->m_pProps->sPar1);
            }
            break;
            case SwServiceType::FieldTypeUser:
            {
                SwFieldType* pFieldType =
                    pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::User, m_pImpl->m_sTypeName, true);
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
                xField.reset(new SwUserField(static_cast<SwUserFieldType*>(pFieldType),
                                    nUserSubType,
                                    m_pImpl->m_pProps->nFormat));
            }
            break;
            case SwServiceType::FieldTypeRefPageSet:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::RefPageSet);
                xField.reset(new SwRefPageSetField( static_cast<SwRefPageSetFieldType*>(pFieldType),
                                    m_pImpl->m_pProps->nUSHORT1,
                                    m_pImpl->m_pProps->bBool1 ));
            }
            break;
            case SwServiceType::FieldTypeRefPageGet:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::RefPageGet);
                SwRefPageGetField *const pRGField = new SwRefPageGetField(
                        static_cast<SwRefPageGetFieldType*>(pFieldType),
                        m_pImpl->m_pProps->nUSHORT1 );
                xField.reset(pRGField);
                pRGField->SetText(m_pImpl->m_pProps->sPar1, nullptr);
            }
            break;
            case SwServiceType::FieldTypePageNum:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::PageNumber);
                SwPageNumberField *const pPNField = new SwPageNumberField(
                    static_cast<SwPageNumberFieldType*>(pFieldType), PG_RANDOM,
                    m_pImpl->m_pProps->nFormat,
                    m_pImpl->m_pProps->nUSHORT1);
                xField.reset(pPNField);
                pPNField->SetUserString(m_pImpl->m_pProps->sPar1);
                uno::Any aVal;
                aVal <<= m_pImpl->m_pProps->nSubType;
                xField->PutValue( aVal, FIELD_PROP_SUBTYPE );
            }
            break;
            case SwServiceType::FieldTypeDDE:
            {
                SwFieldType* pFieldType =
                    pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Dde, m_pImpl->m_sTypeName, true);
                if (!pFieldType)
                    throw uno::RuntimeException();
                xField.reset(new SwDDEField( static_cast<SwDDEFieldType*>(pFieldType) ));
            }
            break;
            case SwServiceType::FieldTypeDatabaseName:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DatabaseName);
                SwDBData aData;
                aData.sDataSource = m_pImpl->m_pProps->sPar1;
                aData.sCommand = m_pImpl->m_pProps->sPar2;
                aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
                xField.reset(new SwDBNameField(static_cast<SwDBNameFieldType*>(pFieldType), aData));
                sal_uInt16  nSubType = xField->GetSubType();
                if (m_pImpl->m_pProps->bBool2)
                    nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
                else
                    nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
                xField->SetSubType(nSubType);
            }
            break;
            case SwServiceType::FieldTypeDatabaseNextSet:
            {
                SwDBData aData;
                aData.sDataSource = m_pImpl->m_pProps->sPar1;
                aData.sCommand = m_pImpl->m_pProps->sPar2;
                aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DbNextSet);
                xField.reset(new SwDBNextSetField(static_cast<SwDBNextSetFieldType*>(pFieldType),
                        m_pImpl->m_pProps->sPar3, aData));
            }
            break;
            case SwServiceType::FieldTypeDatabaseNumSet:
            {
                SwDBData aData;
                aData.sDataSource = m_pImpl->m_pProps->sPar1;
                aData.sCommand = m_pImpl->m_pProps->sPar2;
                aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
                xField.reset(new SwDBNumSetField( static_cast<SwDBNumSetFieldType*>(
                    pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DbNumSet)),
                    m_pImpl->m_pProps->sPar3,
                    OUString::number(m_pImpl->m_pProps->nFormat),
                    aData ));
            }
            break;
            case SwServiceType::FieldTypeDatabaseSetNum:
            {
                SwDBData aData;
                aData.sDataSource = m_pImpl->m_pProps->sPar1;
                aData.sCommand = m_pImpl->m_pProps->sPar2;
                aData.nCommandType = m_pImpl->m_pProps->nSHORT1;
                SwDBSetNumberField *const pDBSNField =
                    new SwDBSetNumberField(static_cast<SwDBSetNumberFieldType*>(
                            pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DbSetNumber)), aData,
                        m_pImpl->m_pProps->nUSHORT1);
                xField.reset(pDBSNField);
                pDBSNField->SetSetNumber(m_pImpl->m_pProps->nFormat);
                sal_uInt16 nSubType = xField->GetSubType();
                if (m_pImpl->m_pProps->bBool2)
                    nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
                else
                    nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
                xField->SetSubType(nSubType);
            }
            break;
            case SwServiceType::FieldTypeDatabase:
            {
                SwFieldType* pFieldType =
                    pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Database, m_pImpl->m_sTypeName, false);
                if (!pFieldType)
                    throw uno::RuntimeException();
                xField.reset(new SwDBField(static_cast<SwDBFieldType*>(pFieldType),
                        m_pImpl->m_pProps->nFormat));
                static_cast<SwDBField*>(xField.get())->InitContent(m_pImpl->m_pProps->sPar1);
                sal_uInt16  nSubType = xField->GetSubType();
                if (m_pImpl->m_pProps->bBool2)
                    nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
                else
                    nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
                xField->SetSubType(nSubType);
            }
            break;
            case SwServiceType::FieldTypeSetExp:
            {
                SwFieldType* pFieldType =
                    pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::SetExp, m_pImpl->m_sTypeName, true);
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
                xField.reset(pSEField);

                sal_uInt16  nSubType = xField->GetSubType();
                if (m_pImpl->m_pProps->bBool2)
                    nSubType &= ~nsSwExtendedSubType::SUB_INVISIBLE;
                else
                    nSubType |= nsSwExtendedSubType::SUB_INVISIBLE;
                if (m_pImpl->m_pProps->bBool3)
                    nSubType |= nsSwExtendedSubType::SUB_CMD;
                else
                    nSubType &= ~nsSwExtendedSubType::SUB_CMD;
                xField->SetSubType(nSubType);
                pSEField->SetSeqNumber(m_pImpl->m_pProps->nUSHORT1);
                pSEField->SetInputFlag(m_pImpl->m_pProps->bBool1);
                pSEField->SetPromptText(m_pImpl->m_pProps->sPar3);
                if (!m_pImpl->m_pProps->sPar4.isEmpty())
                    pSEField->ChgExpStr(m_pImpl->m_pProps->sPar4, nullptr);

            }
            break;
            case SwServiceType::FieldTypeGetExp:
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
                        SwFieldIds::SetExp, m_pImpl->m_pProps->sPar1, false);
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
                            pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::GetExp)),
                        m_pImpl->m_pProps->sPar1, nSubType,
                        m_pImpl->m_pProps->nFormat);
                xField.reset(pGEField);
                //TODO: evaluate SubType!
                if (!m_pImpl->m_pProps->sPar4.isEmpty())
                    pGEField->ChgExpStr(m_pImpl->m_pProps->sPar4, nullptr);
                // #i82544#
                if (bSetGetExpFieldUninitialized)
                    pGEField->SetLateInitialization();
            }
            break;
            case SwServiceType::FieldTypeInputUser:
            case SwServiceType::FieldTypeInput:
            {
                SwFieldType* pFieldType =
                    pDoc->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Input, m_pImpl->m_sTypeName, true);
                if (!pFieldType)
                    throw uno::RuntimeException();
                sal_uInt16 nInpSubType =
                    sal::static_int_cast<sal_uInt16>(
                        SwServiceType::FieldTypeInputUser == m_pImpl->m_nServiceId
                            ? INP_USR : INP_TXT);
                SwInputField * pTextField =
                    new SwInputField(static_cast<SwInputFieldType*>(pFieldType),
                                     m_pImpl->m_pProps->sPar1,
                                     m_pImpl->m_pProps->sPar2,
                                     nInpSubType);
                pTextField->SetHelp(m_pImpl->m_pProps->sPar3);
                pTextField->SetToolTip(m_pImpl->m_pProps->sPar4);

                xField.reset(pTextField);
            }
            break;
            case SwServiceType::FieldTypeMacro:
            {
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Macro);
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
                xField.reset(new SwMacroField(static_cast<SwMacroFieldType*>(pFieldType), aName,
                                        m_pImpl->m_pProps->sPar2));
            }
            break;
            case SwServiceType::FieldTypePageCount:
            case SwServiceType::FieldTypeParagraphCount:
            case SwServiceType::FieldTypeWordCount:
            case SwServiceType::FieldTypeCharacterCount:
            case SwServiceType::FieldTypeTableCount:
            case SwServiceType::FieldTypeGraphicObjectCount:
            case SwServiceType::FieldTypeEmbeddedObjectCount:
            {
                sal_uInt16 nSubType = DS_PAGE;
                switch (m_pImpl->m_nServiceId)
                {
                    case SwServiceType::FieldTypeParagraphCount       : nSubType = DS_PARA; break;
                    case SwServiceType::FieldTypeWordCount            : nSubType = DS_WORD; break;
                    case SwServiceType::FieldTypeCharacterCount       : nSubType = DS_CHAR; break;
                    case SwServiceType::FieldTypeTableCount           : nSubType = DS_TBL;  break;
                    case SwServiceType::FieldTypeGraphicObjectCount  : nSubType = DS_GRF;  break;
                    case SwServiceType::FieldTypeEmbeddedObjectCount : nSubType = DS_OLE;  break;
                    default: break;
                }
                SwFieldType* pFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DocStat);
                xField.reset(new SwDocStatField(
                        static_cast<SwDocStatFieldType*>(pFieldType),
                        nSubType, m_pImpl->m_pProps->nUSHORT2));
            }
            break;
            case SwServiceType::FieldTypeBibliography:
            {
                SwAuthorityFieldType const type(pDoc);
                xField.reset(new SwAuthorityField(static_cast<SwAuthorityFieldType*>(
                            pDoc->getIDocumentFieldsAccess().InsertFieldType(type)),
                        OUString()));
                if (m_pImpl->m_pProps->aPropSeq.hasElements())
                {
                    uno::Any aVal;
                    aVal <<= m_pImpl->m_pProps->aPropSeq;
                    xField->PutValue( aVal, FIELD_PROP_PROP_SEQ );
                }
            }
            break;
            case SwServiceType::FieldTypeCombinedCharacters:
                // create field
                xField.reset(new SwCombinedCharField( static_cast<SwCombinedCharFieldType*>(
                            pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::CombinedChars)),
                            m_pImpl->m_pProps->sPar1));
                break;
            case SwServiceType::FieldTypeDropdown:
            {
                SwDropDownField *const pDDField = new SwDropDownField(
                    static_cast<SwDropDownFieldType *>(
                        pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Dropdown)));
                xField.reset(pDDField);

                pDDField->SetItems(m_pImpl->m_pProps->aStrings);
                pDDField->SetSelectedItem(m_pImpl->m_pProps->sPar1);
                pDDField->SetName(m_pImpl->m_pProps->sPar2);
                pDDField->SetHelp(m_pImpl->m_pProps->sPar3);
                pDDField->SetToolTip(m_pImpl->m_pProps->sPar4);
            }
            break;

            case SwServiceType::FieldTypeTableFormula:
            {
                // create field
                sal_uInt16 nType = nsSwGetSetExpType::GSE_FORMULA;
                if (m_pImpl->m_pProps->bBool1)
                {
                    nType |= nsSwExtendedSubType::SUB_CMD;
                    if (m_pImpl->m_pProps->bFormatIsDefault)
                        m_pImpl->m_pProps->nFormat = -1;
                }
                xField.reset(new SwTableField( static_cast<SwTableFieldType*>(
                    pDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::Table)),
                    m_pImpl->m_pProps->sPar2,
                    nType,
                    m_pImpl->m_pProps->nFormat));
                static_cast<SwTableField*>(xField.get())->ChgExpStr(m_pImpl->m_pProps->sPar1);
            }
            break;
            default: OSL_FAIL("What kind of type is that?");
        }

        if (!xField)
            throw uno::RuntimeException("no SwField created?");

        xField->SetAutomaticLanguage(!m_pImpl->m_pProps->bBool4);
        SwFormatField aFormat(*xField);

        UnoActionContext aCont(pDoc);
        if (aPam.HasMark() &&
            m_pImpl->m_nServiceId != SwServiceType::FieldTypeAnnotation)
        {
            pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
        }

        SwXTextCursor const*const pTextCursor(dynamic_cast<SwXTextCursor*>(pCursor));
        const bool bForceExpandHints(
            pTextCursor
            && pTextCursor->IsAtEndOfMeta() );
        const SetAttrMode nInsertFlags =
            bForceExpandHints
            ? SetAttrMode::FORCEHINTEXPAND
            : SetAttrMode::DEFAULT;

        if (*aPam.GetPoint() != *aPam.GetMark() &&
            m_pImpl->m_nServiceId == SwServiceType::FieldTypeAnnotation)
        {
            // Make sure we always insert the field at the end
            SwPaM aEnd(*aPam.End(), *aPam.End());
            pDoc->getIDocumentContentOperations().InsertPoolItem(aEnd, aFormat, nInsertFlags);
        }
        else
            pDoc->getIDocumentContentOperations().InsertPoolItem(aPam, aFormat, nInsertFlags);

        SwTextAttr* pTextAttr = aPam.GetNode().GetTextNode()->GetFieldTextAttrAt( aPam.GetPoint()->nContent.GetIndex()-1, true );

        // What about updating the fields? (see fldmgr.cxx)
        if (!pTextAttr)
            throw uno::RuntimeException("no SwTextAttr inserted?");  // could theoretically happen, if paragraph is full

        const SwFormatField& rField = pTextAttr->GetFormatField();
        m_pImpl->SetFormatField(const_cast<SwFormatField*>(&rField), pDoc);

        if ( pTextAttr->Which() == RES_TXTATR_ANNOTATION
             && *aPam.GetPoint() != *aPam.GetMark() )
        {
            // create annotation mark
            const SwPostItField* pPostItField = dynamic_cast< const SwPostItField* >(pTextAttr->GetFormatField().GetField());
            OSL_ENSURE( pPostItField != nullptr, "<SwXTextField::attachToRange(..)> - annotation field missing!" );
            if ( pPostItField != nullptr )
            {
                IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess();
                pMarksAccess->makeAnnotationMark( aPam, pPostItField->GetName() );
            }
        }

        xField.reset();

        assert(m_pImpl->GetFormatField());
        m_pImpl->m_pDoc = pDoc;
        m_pImpl->GetFormatField()->SetXTextField(this);
        m_pImpl->m_wThis = *this;
        m_pImpl->m_bIsDescriptor = false;
        m_pImpl->ClearFieldType();
        m_pImpl->m_pProps.reset();
        if (m_pImpl->m_bCallUpdate)
            update();
    }
    else if ( !m_pImpl->IsDescriptor()
              && m_pImpl->m_pDoc != nullptr
              && m_pImpl->m_nServiceId == SwServiceType::FieldTypeAnnotation )
    {
        SwUnoInternalPaM aIntPam( *m_pImpl->m_pDoc );
        if ( !::sw::XTextRangeToSwPaM( aIntPam, xTextRange ) )
            throw lang::IllegalArgumentException();

        // Nothing to do, if the text range has a separate start and end, but they have the same
        // value.
        if (!aIntPam.HasMark() || *aIntPam.Start() != *aIntPam.End())
        {
            UnoActionContext aCont( m_pImpl->m_pDoc );
            // insert copy of annotation at new text range
            std::unique_ptr<SwPostItField> pPostItField(static_cast< SwPostItField* >(m_pImpl->GetFormatField()->GetField()->CopyField().release()));
            SwFormatField aFormatField( *pPostItField );
            pPostItField.reset();
            SwPaM aEnd( *aIntPam.End(), *aIntPam.End() );
            m_pImpl->m_pDoc->getIDocumentContentOperations().InsertPoolItem( aEnd, aFormatField );
            // delete former annotation
            {
                const SwTextField* pTextField = m_pImpl->GetFormatField()->GetTextField();
                SwTextNode& rTextNode = *pTextField->GetpTextNode();
                SwPaM aPam( rTextNode, pTextField->GetStart() );
                aPam.SetMark();
                aPam.Move();
                m_pImpl->m_pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
            }
            // keep inserted annotation
            {
                SwTextField* pTextAttr = aEnd.GetNode().GetTextNode()->GetFieldTextAttrAt( aEnd.End()->nContent.GetIndex()-1, true );
                if ( pTextAttr != nullptr )
                {
                    m_pImpl->SetFormatField(const_cast<SwFormatField*>(&pTextAttr->GetFormatField()), m_pImpl->m_pDoc);

                    if ( *aIntPam.GetPoint() != *aIntPam.GetMark() )
                    {
                        // create annotation mark
                        const SwPostItField* pField = dynamic_cast< const SwPostItField* >(pTextAttr->GetFormatField().GetField());
                        OSL_ENSURE( pField != nullptr, "<SwXTextField::attach(..)> - annotation field missing!" );
                        if ( pField != nullptr )
                        {
                            IDocumentMarkAccess* pMarksAccess = aIntPam.GetDoc().getIDocumentMarkAccess();
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

uno::Reference< text::XTextRange > SAL_CALL
SwXTextField::getAnchor()
{
    SolarMutexGuard aGuard;

    SwField const*const pField = m_pImpl->GetField();
    if (!pField)
        return nullptr;

    const SwTextField* pTextField = m_pImpl->GetFormatField()->GetTextField();
    if (!pTextField)
        throw uno::RuntimeException();

    std::shared_ptr< SwPaM > pPamForTextField;
    SwTextField::GetPamForTextField(*pTextField, pPamForTextField);
    if (pPamForTextField == nullptr)
        return nullptr;

    // If this is a postit field, then return the range of its annotation mark if it has one.
    if (pField->Which() == SwFieldIds::Postit)
    {
        const SwPostItField* pPostItField = static_cast<const SwPostItField*>(pField);
        IDocumentMarkAccess* pMarkAccess = m_pImpl->m_pDoc->getIDocumentMarkAccess();
        for (IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAnnotationMarksBegin(); ppMark != pMarkAccess->getAnnotationMarksEnd(); ++ppMark)
        {
            if ((*ppMark)->GetName() == pPostItField->GetName())
            {
                pPamForTextField = std::make_shared<SwPaM>((*ppMark)->GetMarkStart(), (*ppMark)->GetMarkEnd());
                break;
            }
        }
    }

    uno::Reference<text::XTextRange> xRange = SwXTextRange::CreateXTextRange(
            *m_pImpl->m_pDoc, *(pPamForTextField->GetPoint()), pPamForTextField->GetMark());
    return xRange;
}

void SAL_CALL SwXTextField::dispose()
{
    SolarMutexGuard aGuard;
    SwField const*const pField = m_pImpl->GetField();
    if(pField && m_pImpl->m_pDoc)
    {
        UnoActionContext aContext(m_pImpl->m_pDoc);
        assert(m_pImpl->GetFormatField()->GetTextField() && "<SwXTextField::dispose()> - missing <SwTextField> --> crash");
        SwTextField::DeleteTextField(*(m_pImpl->GetFormatField()->GetTextField()));
    }

    if (m_pImpl->m_xTextObject.is())
    {
        m_pImpl->m_xTextObject->DisposeEditSource();
        m_pImpl->m_xTextObject.clear();
    }
    m_pImpl->Invalidate();
}

void SAL_CALL SwXTextField::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXTextField::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL
SwXTextField::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    // no static
    uno::Reference< beans::XPropertySetInfo >  aRef;
    if (m_pImpl->m_nServiceId == SwServiceType::Invalid)
    {
        throw uno::RuntimeException();
    }
    const SfxItemPropertySet* pPropSet = aSwMapProvider.GetPropertySet(
                    lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    const uno::Reference<beans::XPropertySetInfo>& xInfo = pPropSet->getPropertySetInfo();
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
{
    SolarMutexGuard aGuard;
    SwField const*const pField = m_pImpl->GetField();
    const SfxItemPropertySet* _pPropSet = aSwMapProvider.GetPropertySet(
                lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    const SfxItemPropertyMapEntry*   pEntry = _pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
        throw beans::PropertyVetoException( "Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pField)
    {
        // special treatment for mail merge fields
        const SwFieldIds nWhich = pField->Which();
        if( SwFieldIds::Database == nWhich &&
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
            const SwTextField* pTextField = m_pImpl->GetFormatField()->GetTextField();
            if(!pTextField)
                throw uno::RuntimeException();
            SwPosition aPosition( pTextField->GetTextNode() );
            aPosition.nContent = pTextField->GetStart();
            pDoc->getIDocumentFieldsAccess().PutValueToField( aPosition, rValue, pEntry->nWID);
        }

        //#i100374# notify SwPostIt about new field content
        assert(m_pImpl->GetFormatField());
        if (SwFieldIds::Postit == nWhich)
        {
            m_pImpl->GetFormatField()->Broadcast(
                    SwFormatFieldHint( nullptr, SwFormatFieldHintWhich::CHANGED ));
        }

        // fdo#42073 notify SwTextField about changes of the expanded string
        if (m_pImpl->GetFormatField()->GetTextField())
        {
            m_pImpl->GetFormatField()->GetTextField()->ExpandTextField();
        }

        //#i100374# changing a document field should set the modify flag
        SwDoc* pDoc = m_pImpl->m_pDoc;
        if (pDoc)
            pDoc->getIDocumentState().SetModified();

    }
    else if (m_pImpl->m_pProps)
    {
        bool* pBool = nullptr;
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
            auto aTemp = o3tl::tryAccess<util::Date>(rValue);
            if(!aTemp)
                throw lang::IllegalArgumentException();

            m_pImpl->m_pProps->aDate = Date(aTemp->Day, aTemp->Month, aTemp->Year);
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
                m_pImpl->m_pProps->pDateTime.reset( new util::DateTime );
            rValue >>= *m_pImpl->m_pProps->pDateTime;
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
            auto b = o3tl::tryAccess<bool>(rValue);
            if( !b )
                throw lang::IllegalArgumentException();
            *pBool = *b;

        }
    }
    else
        throw uno::RuntimeException();
}

uno::Any SAL_CALL SwXTextField::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwField const*const pField = m_pImpl->GetField();
    const SfxItemPropertySet* _pPropSet = aSwMapProvider.GetPropertySet(
                lcl_GetPropertyMapOfService(m_pImpl->m_nServiceId));
    const SfxItemPropertyMapEntry*   pEntry = _pPropSet->getPropertyMap().getByName(rPropertyName);
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
            uno::Sequence<text::TextContentAnchorType> aTypes { text::TextContentAnchorType_AS_CHARACTER };
            aRet <<= aTypes;
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
                SwViewShell *pViewShell = nullptr;
                SwEditShell *pEditShell = nullptr;
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
                    (m_pImpl->GetField()) ? m_pImpl->GetFormatField() : nullptr;
                const SwTextField* pTextField = pFieldFormat
                    ? m_pImpl->GetFormatField()->GetTextField() : nullptr;
                if(!pTextField)
                    throw uno::RuntimeException();
                const SwTextNode& rTextNode = pTextField->GetTextNode();

                // skip fields that are currently not in the document
                // e.g. fields in undo or redo array
                if (rTextNode.GetNodes().IsDocNodes())
                {
                    bool bFrame = 0 != rTextNode.FindLayoutRect().Width(); // or so
                    bool bHidden = rTextNode.IsHidden();
                    if ( !bHidden )
                    {
                        sal_Int32 nHiddenStart;
                        sal_Int32 nHiddenEnd;
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
                    if (!m_pImpl->m_xTextObject.is())
                    {
                        m_pImpl->m_xTextObject
                            = new SwTextAPIObject( std::make_unique<SwTextAPIEditSource>(m_pImpl->m_pDoc) );
                    }

                    uno::Reference<text::XText> xText(m_pImpl->m_xTextObject);
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
                aRet <<= m_pImpl->m_pProps->aDate.GetUNODate();
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
                    aRet <<= *m_pImpl->m_pProps->pDateTime;
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

void SwXTextField::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextField::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextField::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextField::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextField::update()
{
    SolarMutexGuard aGuard;
    SwField * pField = const_cast<SwField*>(m_pImpl->GetField());
    if (pField)
    {
        switch(pField->Which())
        {
            case SwFieldIds::DateTime:
            static_cast<SwDateTimeField*>(pField)->SetDateTime( ::DateTime( ::DateTime::SYSTEM ) );
            break;

            case SwFieldIds::ExtUser:
            {
                SwExtUserField* pExtUserField = static_cast<SwExtUserField*>(pField);
                pExtUserField->SetExpansion( SwExtUserFieldType::Expand(
                                            pExtUserField->GetSubType() ) );
            }
            break;

            case SwFieldIds::Author:
            {
                SwAuthorField* pAuthorField = static_cast<SwAuthorField*>(pField);
                pAuthorField->SetExpansion( SwAuthorFieldType::Expand(
                                            pAuthorField->GetFormat() ) );
            }
            break;

            case SwFieldIds::Filename:
            {
                SwFileNameField* pFileNameField = static_cast<SwFileNameField*>(pField);
                pFileNameField->SetExpansion( static_cast<SwFileNameFieldType*>(pField->GetTyp())->Expand(
                                            pFileNameField->GetFormat() ) );
            }
            break;

            case SwFieldIds::DocInfo:
            {
                    SwDocInfoField* pDocInfField = static_cast<SwDocInfoField*>(pField);
                    pDocInfField->SetExpansion( static_cast<SwDocInfoFieldType*>(pField->GetTyp())->Expand(
                                                pDocInfField->GetSubType(),
                                                pDocInfField->GetFormat(),
                                                pDocInfField->GetLanguage(),
                                                pDocInfField->GetName() ) );
            }
            break;
            default: break;
        }
        // Text formatting has to be triggered.
        m_pImpl->GetFormatField()->UpdateTextNode(nullptr, nullptr);
    }
    else
        m_pImpl->m_bCallUpdate = true;
}

OUString SAL_CALL SwXTextField::getImplementationName()
{
    return "SwXTextField";
}

static OUString OldNameToNewName_Impl( const OUString &rOld )
{
    static const char aOldNamePart1[] = ".TextField.DocInfo.";
    static const char aOldNamePart2[] = ".TextField.";
    OUString sServiceNameCC( rOld );
    sal_Int32 nIdx = sServiceNameCC.indexOf( aOldNamePart1 );
    if (nIdx >= 0)
        sServiceNameCC = sServiceNameCC.replaceAt( nIdx, strlen(aOldNamePart1), u".textfield.docinfo." );
    nIdx = sServiceNameCC.indexOf( aOldNamePart2 );
    if (nIdx >= 0)
        sServiceNameCC = sServiceNameCC.replaceAt( nIdx, strlen(aOldNamePart2), u".textfield." );
    return sServiceNameCC;
}

sal_Bool SAL_CALL SwXTextField::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SwXTextField::getSupportedServiceNames()
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
    EndListeningAll();
    m_pFormatField = nullptr;
    m_pDoc = nullptr;
    uno::Reference<uno::XInterface> const xThis(m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_EventListeners.disposeAndClear(ev);
}

void SwXTextField::Impl::Notify(const SfxHint& rHint)
{

    if(rHint.GetId() == SfxHintId::Dying)
        Invalidate();
    else if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
        switch(pLegacyHint->m_pOld ? pLegacyHint->m_pOld->Which() : 0)
        {
            case RES_REMOVE_UNO_OBJECT:
            case RES_OBJECTDYING:
                Invalidate();
                break;
        }
    }
}

const SwField* SwXTextField::Impl::GetField() const
{
    return m_pFormatField ? m_pFormatField->GetField() : nullptr;
}

OUString SwXTextFieldMasters::getImplementationName()
{
    return "SwXTextFieldMasters";
}

sal_Bool SwXTextFieldMasters::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextFieldMasters::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.TextFieldMasters" };
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
static SwFieldIds lcl_GetIdByName( OUString& rName, OUString& rTypeName )
{
    if (rName.startsWithIgnoreAsciiCase(COM_TEXT_FLDMASTER_CC))
        rName = rName.copy(COM_TEXT_FLDMASTER_CC.getLength());

    SwFieldIds nResId = SwFieldIds::Unknown;
    sal_Int32 nIdx = 0;
    rTypeName = rName.getToken( 0, '.', nIdx );
    if (rTypeName == "User")
        nResId = SwFieldIds::User;
    else if (rTypeName == "DDE")
        nResId = SwFieldIds::Dde;
    else if (rTypeName == "SetExpression")
    {
        nResId = SwFieldIds::SetExp;

        const OUString sFieldTypName( rName.getToken( 0, '.', nIdx ));
        const OUString sUIName( SwStyleNameMapper::GetSpecialExtraUIName( sFieldTypName ) );

        if( sUIName != sFieldTypName )
            rName = comphelper::string::setToken(rName, 1, '.', sUIName);
    }
    else if (rTypeName.equalsIgnoreAsciiCase("DataBase"))
    {
        rName = rName.copy(RTL_CONSTASCII_LENGTH("DataBase."));
        if (!rName.isEmpty())
        {
            // #i51815#
            rName = "DataBase." + rName;
            nResId = SwFieldIds::Database;
        }
    }
    else if (rTypeName == "Bibliography")
        nResId = SwFieldIds::TableOfAuthorities;
    return nResId;
}

uno::Any SwXTextFieldMasters::getByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    OUString sName(rName), sTypeName;
    const SwFieldIds nResId = lcl_GetIdByName( sName, sTypeName );
    if( SwFieldIds::Unknown == nResId )
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
    case SwFieldIds::User:
        sField = "User." + rFieldType.GetName();
        break;
    case SwFieldIds::Dde:
        sField = "DDE." + rFieldType.GetName();
        break;

    case SwFieldIds::SetExp:
        sField = "SetExpression." + SwStyleNameMapper::GetSpecialExtraProgName( rFieldType.GetName() );
        break;

    case SwFieldIds::Database:
        sField = "DataBase." + rFieldType.GetName().replaceAll(OUStringChar(DB_DELIM), ".");
        break;

    case SwFieldIds::TableOfAuthorities:
        sField = "Bibliography";
        break;

    default:
        return false;
    }

    rName += COM_TEXT_FLDMASTER_CC + sField;
    return true;
}

uno::Sequence< OUString > SwXTextFieldMasters::getElementNames()
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

    return comphelper::containerToSequence(aFieldNames);
}

sal_Bool SwXTextFieldMasters::hasByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw uno::RuntimeException();

    OUString sName(rName), sTypeName;
    const SwFieldIds nResId = lcl_GetIdByName( sName, sTypeName );
    bool bRet = false;
    if( SwFieldIds::Unknown != nResId )
    {
        sName = sName.copy(std::min(sTypeName.getLength()+1, sName.getLength()));
        bRet = nullptr != GetDoc()->getIDocumentFieldsAccess().GetFieldType(nResId, sName, true);
    }
    return bRet;
}

uno::Type  SwXTextFieldMasters::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();

}

sal_Bool SwXTextFieldMasters::hasElements()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return true;
}

class SwXTextFieldTypes::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper3

public:
    ::comphelper::OInterfaceContainerHelper3<util::XRefreshListener> m_RefreshListeners;

    Impl() : m_RefreshListeners(m_Mutex) { }
};

OUString SwXTextFieldTypes::getImplementationName()
{
    return "SwXTextFieldTypes";
}

sal_Bool SwXTextFieldTypes::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextFieldTypes::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet { "com.sun.star.text.TextFields" };
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
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return new SwXFieldEnumeration(*GetDoc());
}

uno::Type  SwXTextFieldTypes::getElementType()
{
    return cppu::UnoType<text::XDependentTextField>::get();
}

sal_Bool SwXTextFieldTypes::hasElements()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw uno::RuntimeException();
    return true; // they always exist
}

void SAL_CALL SwXTextFieldTypes::refresh()
{
    {
        SolarMutexGuard aGuard;
        if (!IsValid())
            throw uno::RuntimeException();
        UnoActionContext aContext(GetDoc());
        GetDoc()->getIDocumentStatistics().UpdateDocStat( false, true );
        GetDoc()->getIDocumentFieldsAccess().UpdateFields(false);
    }
    // call refresh listeners (without SolarMutex locked)
    lang::EventObject const event(static_cast< ::cppu::OWeakObject*>(this));
    m_pImpl->m_RefreshListeners.notifyEach(
            & util::XRefreshListener::refreshed, event);
}

void SAL_CALL SwXTextFieldTypes::addRefreshListener(
        const uno::Reference<util::XRefreshListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.addInterface(xListener);
}

void SAL_CALL SwXTextFieldTypes::removeRefreshListener(
        const uno::Reference<util::XRefreshListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.removeInterface(xListener);
}

class SwXFieldEnumeration::Impl
    : public SvtListener
{
public:
    SwDoc* m_pDoc;
    std::vector<uno::Reference<text::XTextField>> m_Items;
    sal_Int32 m_nNextIndex;  ///< index of next element to be returned

    explicit Impl(SwDoc& rDoc)
        : m_pDoc(&rDoc)
        , m_nNextIndex(0)
    {
        StartListening(rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->GetNotifier());
    }

    virtual void Notify(const SfxHint& rHint) override
    {
        if(rHint.GetId() == SfxHintId::Dying)
            m_pDoc = nullptr;
    }
};

OUString SAL_CALL
SwXFieldEnumeration::getImplementationName()
{
    return "SwXFieldEnumeration";
}

sal_Bool SAL_CALL SwXFieldEnumeration::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL
SwXFieldEnumeration::getSupportedServiceNames()
{
    return { "com.sun.star.text.FieldEnumeration" };
}

SwXFieldEnumeration::SwXFieldEnumeration(SwDoc & rDoc)
    : m_pImpl(new Impl(rDoc))
{
    // build sequence
    m_pImpl->m_Items.clear();

    const SwFieldTypes* pFieldTypes = m_pImpl->m_pDoc->getIDocumentFieldsAccess().GetFieldTypes();
    const size_t nCount = pFieldTypes->size();
    for(size_t nType = 0;  nType < nCount;  ++nType)
    {
        const SwFieldType* pCurType = (*pFieldTypes)[nType].get();
        std::vector<SwFormatField*> vFormatFields;
        pCurType->GatherFields(vFormatFields);
        std::for_each(vFormatFields.begin(), vFormatFields.end(),
                [this](SwFormatField* pF) { m_pImpl->m_Items.push_back(SwXTextField::CreateXTextField(m_pImpl->m_pDoc, pF)); });
    }
    // now handle meta-fields, which are not SwFields
    const std::vector< uno::Reference<text::XTextField> > MetaFields(
           m_pImpl->m_pDoc->GetMetaFieldManager().getMetaFields() );
    for (const auto & rMetaField : MetaFields)
    {
        m_pImpl->m_Items.push_back( rMetaField );
    }
    // also add fieldmarks
    IDocumentMarkAccess& rMarksAccess(*rDoc.getIDocumentMarkAccess());
    for (auto iter = rMarksAccess.getFieldmarksBegin(); iter != rMarksAccess.getFieldmarksEnd(); ++iter)
    {
        m_pImpl->m_Items.emplace_back(SwXFieldmark::CreateXFieldmark(rDoc, *iter), uno::UNO_QUERY);
    }
}

SwXFieldEnumeration::~SwXFieldEnumeration()
{
}

sal_Bool SAL_CALL SwXFieldEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;

    return m_pImpl->m_nNextIndex < static_cast<sal_Int32>(m_pImpl->m_Items.size());
}

uno::Any SAL_CALL SwXFieldEnumeration::nextElement()
{
    SolarMutexGuard aGuard;

    if (m_pImpl->m_nNextIndex >= static_cast<sal_Int32>(m_pImpl->m_Items.size()))
        throw container::NoSuchElementException(
            "SwXFieldEnumeration::nextElement",
            css::uno::Reference<css::uno::XInterface>());

    uno::Reference< text::XTextField >  &rxField =
        m_pImpl->m_Items[ m_pImpl->m_nNextIndex++ ];
    uno::Any aRet;
    aRet <<= rxField;
    rxField = nullptr;  // free memory for item that is no longer used
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
