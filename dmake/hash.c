/* RCS  $Id: hash.c,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Hashing function for hash tables.
--
-- DESCRIPTION
--      Hash an identifier.  The hashing function works by computing the sum
--      of each char and the previous hash value multiplied by 129.  Finally the
--      length of the identifier is added in.  This way the hash depends on the
--      chars as well as the length, and appears to be sufficiently unique,
--      and is FAST to COMPUTE, unlike the previous hash function...
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/

#include "extern.h"

PUBLIC uint16
Hash( id, phv )/*
=================
      This function computes the identifier's hash value and returns the hash
      value modulo the key size as well as the full hash value.  The reason
      for returning both is so that hash table searches can be sped up.  You
      compare hash keys instead and compare strings only for those whose 32-bit
      hash keys match. (not many) */

char   *id;
uint32 *phv;
{
   register char   *p    = id;
   register uint32 hash  = (uint32) 0;

   while( *p ) hash = (hash << 7) + hash + (uint32) (*p++);
   *phv = hash = hash + (uint32) (p-id);

   return( (uint16) (hash % HASH_TABLE_SIZE) );
}

