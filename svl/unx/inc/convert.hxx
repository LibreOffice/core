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


#ifndef _CONVERT_HXX
#define _CONVERT_HXX

/*
#define _SWAPSHORT(x) ((((x) & 0xFF00)>>8) | (((x) & 0x00FF)<<8))
#define _SWAPLONG(x)  ((((x) & 0xFF000000)>>24) | (((x) & 0x00FF0000)>>8) |  \
                      (((x) & 0x0000FF00)<<8) | (((x) & 0x000000FF)<<24))
*/
class Convert
{
public:
    static void     Swap( long & nValue )
                    { nValue = SWAPLONG( nValue ); }
    static void     Swap( ULONG & nValue )
                    { nValue = SWAPLONG( nValue ); }
    static void     Swap( short & nValue )
                    { nValue = SWAPSHORT( nValue ); }
    static void     Swap( USHORT & nValue )
                    { nValue = SWAPSHORT( nValue ); }
    static void     Swap( Point & aPtr )
                    { Swap( aPtr.X() ); Swap( aPtr.Y() ); }
    static void     Swap( Size & aSize )
                    { Swap( aSize.Width() ); Swap( aSize.Height() ); }
    static void     Swap( Rectangle & rRect )
                    { Swap( rRect.Top() ); Swap( rRect.Bottom() );
                      Swap( rRect.Left() ); Swap( rRect.Right() ); }
/*
    static USHORT   AnsiFloatSize() const { return 6; }
    static float    AnsiToFloat( void * pAnsiFloat )
                    { return 0; }
    static USHORT   AnsiDoubleSize() const { return 12; }
    static double   AnsiToDouble( void * pAnsiDouble )
                    { return 0; }
*/
};

#endif // _CONVERT_HXX
