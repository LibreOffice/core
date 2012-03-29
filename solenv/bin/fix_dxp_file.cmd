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
/* os2 build scripts

this script is used to process dxp files produced from .map

*/

do while( lines())

	l = linein()

	l = strip(l)
	l = strip(l,,X2C(9))
	l = strip(l,,";")
	if LEFT( l,4) \= 'Java' THEN l = '_'l

	/* remove empty lines */
	if l = '_' then l = ''

	/* remove component_getDescriptionFunc, since it is already added by tg_def */
	if l = '_component_getDescriptionFunc' then l = ''
	if l = '_GetVersionInfo' then l = ''

	/* remove GLOBAL symbols */
/*
	if WORDPOS( l, '_GLOBAL_') > 0 then l = ''
*/

	say l

end
