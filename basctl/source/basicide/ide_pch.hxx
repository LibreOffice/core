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


#include <svheader.hxx>

#define ITEMID_SEARCH           0
#define ITEMID_SIZE             0

#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/menu.hxx>

#include <svl/svarray.hxx>
#include <svl/itemset.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <svl/srchitem.hxx>

#define _BASIC_TEXTPORTIONS
#include <basic/sbdef.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/basmgr.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/event.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/progress.hxx>
#include <editeng/sizeitem.hxx>


