/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtl/random.h>

static char *pt (unsigned char *md, int length)
{
    int i;
    static char buf[80];

    for (i=0; i<length; i++)
        sprintf(&(buf[i*2]),"%02x",md[i]);

    return(buf);
}

/*
 * main.
 */
#ifdef WIN32
int __cdecl main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
    rtlRandomPool pool;
    pool = rtl_random_createPool();
    if (pool)
    {
        unsigned char buffer[1000];

        rtl_random_getBytes (pool, buffer, 8);
        printf ("random: %s\n", pt (buffer, 8));
    }
    rtl_random_destroyPool (pool);
    return(0);
}

