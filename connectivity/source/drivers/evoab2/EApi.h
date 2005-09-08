 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EApi.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:49:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_EVOLUTION_API_HXX_
#define _CONNECTIVITY_EVOAB_EVOLUTION_API_HXX_
#include <glib-object.h>

// Initializes the API below, returns false if not available
bool EApiInit();

G_BEGIN_DECLS

// This header defined all the API methods as
// function pointers instead of real functions
// this will all evaporate as it is compiled generating
// no symbol lookups or relocations, but giving code
// clarity.

// We attempt to define a minimum API that we use:

// e-contact.h
typedef void EContact;
#define E_CONTACT(a) ((EContact *)(a))
#define E_TYPE_CONTACT (e_contact_get_type())
typedef int EContactField;

// e-source.h
typedef void ESource;
#define E_SOURCE(a) ((ESource *)(a))
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
const char  *(*e_source_peek_name)    (ESource *source);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
const gchar *(*e_source_get_property) (ESource *source,
                       const gchar *property);

#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
GType         (*e_contact_get_type) (void);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EContactField (*e_contact_field_id) (const char *field_name);

// e-source-list.h
typedef void ESourceList;
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
GSList *(*e_source_list_peek_groups) (ESourceList *list);

// e-source-group.h
typedef void ESourceGroup;
#define E_SOURCE_GROUP(a) ((ESourceGroup *)(a))

#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
GSList  *(*e_source_group_peek_sources) (ESourceGroup *group);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
const char *(*e_source_group_peek_base_uri)  (ESourceGroup *group);
// e-book.h
typedef enum {
  E_BOOK_QUERY_IS,
  E_BOOK_QUERY_CONTAINS,
  E_BOOK_QUERY_BEGINS_WITH,
  E_BOOK_QUERY_ENDS_WITH,
} EBookQueryTest;

typedef void EBook;
typedef void EBookQuery;

#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EBook      *(*e_book_new)                     (ESource      *source,
                           GError      **error);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
gboolean    (*e_book_open)                    (EBook       *book,
                           gboolean     only_if_exists,
                           GError     **error);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
const char *(*e_book_get_uri)                 (EBook        *book);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
ESource    *(*e_book_get_source)              (EBook        *book);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
gboolean    (*e_book_get_addressbooks)        (ESourceList **addressbook_sources,
                           GError      **error);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
gboolean    (*e_book_get_contacts)            (EBook        *book,
                           EBookQuery   *query,
                           GList       **contacts,
                           GError      **error);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
gboolean    (*e_book_authenticate_user)       (EBook        *book,
                           const char   *user,
                           const char   *passwd,
                           const char   *auth_method,
                           GError      **error);

// e-book-query.h
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EBookQuery* (*e_book_query_field_exists) (EContactField   field);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EBookQuery* (*e_book_query_field_test)   (EContactField   field,
                      EBookQueryTest     test,
                      const char        *value);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EBookQuery* (*e_book_query_and)          (int nqs, EBookQuery **qs, gboolean unref);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EBookQuery* (*e_book_query_or)           (int nqs, EBookQuery **qs, gboolean unref);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
EBookQuery* (*e_book_query_not)          (EBookQuery *q, gboolean unref);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
void        (*e_book_query_unref)        (EBookQuery *q);
#ifndef _EVOLUTION_ALREADY_DEFINED_
extern
#endif
char*       (*e_book_query_to_string)    (EBookQuery *q);
G_END_DECLS
#endif
