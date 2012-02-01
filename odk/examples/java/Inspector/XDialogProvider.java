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



public interface XDialogProvider {

    public static String SINVOKE = "Invoke";

    public static String SADDTOSOURCECODE = "Add to Sourcecode";

    public void enablePopupMenuItem(String _sMenuTitle, boolean _bdoEnable);

    public void showPopUpMenu(Object invoker, int x, int y) throws java.lang.ClassCastException;

    public InspectorPane getSelectedInspectorPage();

    public void addInspectorPage(String _sTitle, Object _oContainer);

    public InspectorPane getInspectorPage(int _nIndex);

    public int getInspectorPageCount();

    public void selectInspectorPageByIndex(int nTabIndex);

    public void show(int _nPageIndex) throws java.lang.ClassCastException;

    public void selectSourceCodeLanguage(int _nLanguage);

    public void paint();

    // returns one of the constants defined in XLanguageSourceCodeGenerator
    public int getLanguage();

    public String getIDLPath();
}
