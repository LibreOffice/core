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

#pragma once


/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing src files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
       400         /* #include | #pragma | //... | ... */
             401         /*...                               */
          402         /* Text = {                         */
            404         /* XYZ                              */
         407         /* .                                */
/*------------------------------------------------------                    */
/* prev. tokens will not be executed                                        */
        500
/* following tokens will be executed                                        */
/*------------------------------------------------------                    */
           501         /* #if... | #endif ... | ...        */
           502         /*                                  */
            503         /* Menu MID_TEST                    */
        504         /* Menu ( MID_TEST + .. )           */
        505         /* PageItem {                       */
            506         /* TEXT = "hhh"                     */
        507         /* TEXT = "hhh" TEST "HHH" ...      */
                508         /* "Something like this"            */
             509         /* {                                */
           510         /* };                               */
      511         /* MAP_APPFONT(10,10)               */
          512         /* Something = Anything             */
      513         /* ...List [xyz]=...                */
            514         /* < "Text" ... >                   */
           515         /*                   */
       516         /*                                  */
              519         /* #pragma ...                      */
           521         /* { "Text" ... }                   */
          524         /* #define ...                      */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing cfg files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
                     501
              505
                506
                507
                508
              509
           600
         601
        602
          603
           604
          605
      606

/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing xrm files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
              507
                508
                600

/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing description.xml files                                 */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
          700
            701
                  702
                    703
 704
   705
   706

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
