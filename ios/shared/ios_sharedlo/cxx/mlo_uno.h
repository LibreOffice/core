// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef __LibreOffice_mlo_uno__
#define __LibreOffice_mlo_uno__

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {MARK_ALL,MARK_FIRST,MARK_NEXT,MARK_PREVIOUS} MLOFindSelectionType;
    typedef enum {LAST_PAGE,CURRENT_PAGE} MLOGetPageType;

    BOOL mlo_is_document_open(void);
    BOOL mlo_open_file(NSString * file);
    void mlo_close(void);
    long mlo_get_page_count(void);
    long mlo_get_current_page(void);
    int mlo_find(NSString * toSearch,MLOFindSelectionType type);
    void mlo_get_selection(NSMutableString * mutableString);
    void mlo_fetch_view_data(NSMutableString * mutableString);
    void mlo_select_all(void);
    void mlo_save(void);

#ifdef __cplusplus
}
#endif
#endif /* defined(__LibreOffice_mlo_uno__) */
