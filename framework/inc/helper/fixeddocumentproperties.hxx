/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __FRAMEWORK_HELPER_FIXEDDOCUMENTPROPERTIES_HXX_
#define __FRAMEWORK_HELPER_FIXEDDOCUMENTPROPERTIES_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/DateTime.hpp>

namespace framework
{

struct FixedDocumentProperties
{
    // fixed Properties (sort by name and used by OPropertySetHelper)
    ::rtl::OUString                                 m_sAuthor                   ;   /// Creator of this document
    sal_Bool                                        m_bAutoloadEnabled          ;   /// Automatic reload enabled/disabled ?
    sal_Int32                                       m_nAutoloadSecs             ;   /// Time cycle for automatic reload
    ::rtl::OUString                                 m_sAutoloadURL              ;   /// URL for automatic reload
    ::rtl::OUString                                 m_sBlindCopiesTo            ;   /// Adress for BCC
    ::rtl::OUString                                 m_sCopiesTo                 ;   /// Adress CC
    com::sun::star::util::DateTime                  m_aCreationDate             ;   /// Time and date of document creation
    ::rtl::OUString                                 m_sDefaultTarget            ;   /// Name of default target
    ::rtl::OUString                                 m_sDescription              ;   /// Short description and comments
    sal_Int16                                       m_nEditingCycles            ;   /// Document number
    sal_Int32                                       m_nEditingDuration          ;   /// Duration since last edit
    com::sun::star::uno::Sequence< sal_Int8 >       m_seqExtraData              ;   /// Buffer of bytes for some extra data
    ::rtl::OUString                                 m_sInReplyTo                ;   /// Adress for reply to ...
    sal_Bool                                        m_bIsEncrypted              ;   /// Document is en/decrypted
    ::rtl::OUString                                 m_sKeywords                 ;   /// Some keywords
    ::rtl::OUString                                 m_sMIMEType                 ;   /// MIME-type of document
    ::rtl::OUString                                 m_sModifiedBy               ;   /// Name of user who has modified as last one
    com::sun::star::util::DateTime                  m_aModifyDate               ;   /// Date and time of last modification
    ::rtl::OUString                                 m_sNewsgroups               ;   /// Document is a message of newsgroup
    ::rtl::OUString                                 m_sOriginal                 ;   /// Message ID
    sal_Bool                                        m_bPortableGraphics         ;   /// Use of portable graphics
    com::sun::star::util::DateTime                  m_aPrintDate                ;   /// Date and time of last print
    ::rtl::OUString                                 m_sPrintedBy                ;   /// Name of user who has printed as last one
    sal_uInt16                                      m_nPriority                 ;   /// Priority of message, if document a message!
    sal_Bool                                        m_bQueryTemplate            ;   /// Search for template ?
    ::rtl::OUString                                 m_sRecipient                ;   /// Name of recipient
    ::rtl::OUString                                 m_sReferences               ;   /// Some references to other things
    ::rtl::OUString                                 m_sReplyTo                  ;   /// Adress to reply
    sal_Bool                                        m_bSaveGraphicsCompressed   ;   /// Compressed/non compressed graphics
    sal_Bool                                        m_bSaveOriginalGraphics     ;   /// Include graphics
    sal_Bool                                        m_bSaveVersionOnClose       ;   /// Save version on close of document ?
    ::rtl::OUString                                 m_sTemplate                 ;   /// Name of template
    sal_Bool                                        m_bTemplateConfig           ;   /// Exist template config ?
    com::sun::star::util::DateTime                  m_aTemplateDate             ;   /// Date and time of template
    ::rtl::OUString                                 m_sTemplateFileName         ;   /// File name of template
    ::rtl::OUString                                 m_sTheme                    ;   /// Name of theme
    ::rtl::OUString                                 m_sTitle                    ;   /// Title of document
    sal_Bool                                        m_bUserData                 ;   /// Exist user data ?
};

}

#endif // __FRAMEWORK_SERVICES_FIXEDDOCUMENTPROPERTIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
