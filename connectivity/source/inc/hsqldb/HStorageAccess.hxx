/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HStorageAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:14:59 $
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

#ifndef CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX
#define CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX

#include "hsqldb/HStorageAccess.h"

namespace connectivity { namespace hsqldb
{
    class DataLogFile;
} }

jint read_from_storage_stream( JNIEnv * env, jobject obj_this, jstring name, jstring key, ::connectivity::hsqldb::DataLogFile* logger = NULL );
jint read_from_storage_stream_into_buffer( JNIEnv * env, jobject obj_this,jstring name, jstring key, jbyteArray buffer, jint off, jint len, ::connectivity::hsqldb::DataLogFile* logger = NULL );
void write_to_storage_stream_from_buffer( JNIEnv* env, jobject obj_this, jstring name, jstring key, jbyteArray buffer, jint off, jint len, ::connectivity::hsqldb::DataLogFile* logger = NULL );
void write_to_storage_stream( JNIEnv* env, jobject obj_this, jstring name, jstring key, jint v, ::connectivity::hsqldb::DataLogFile* logger = NULL );

#endif // CONNECTIVITY_HSQLDB_STORAGEACCESS_HXX
