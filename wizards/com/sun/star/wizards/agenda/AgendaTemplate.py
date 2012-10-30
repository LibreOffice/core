#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import traceback
import uno
from .TemplateConsts import *
from threading import RLock
from ..text.TextDocument import TextDocument
from ..common.FileAccess import FileAccess
from ..common.TextElement import TextElement
from ..common.Helper import Helper
from ..text.TextSectionHandler import TextSectionHandler

from datetime import date as dateTimeObject

from com.sun.star.text.PlaceholderType import TEXT
from com.sun.star.i18n.NumberFormatIndex import TIME_HHMM, DATE_SYSTEM_LONG

def synchronized(lock):
    ''' Synchronization decorator. '''
    def wrap(f):
        def newFunction(*args, **kw):
            lock.acquire()
            try:
                return f(*args, **kw)
            finally:
                lock.release()
        return newFunction
    return wrap

'''
The classes here implement the whole document-functionality of the agenda wizard:
the live-preview and the final "creation" of the document,
when the user clicks "finish". <br/>
<br/>
<h2>Some terminology:<h2/>
items are names or headings. we don't make any distinction.

<br/>
The Agenda Template is used as general "controller"
of the whole document, whereas the two child-classes ItemsTable
and TopicsTable control the item tables (note plural!) and the
topics table (note singular).<br/>
<br/>
Other small classes are used to abstract the handling of cells and text and we
try to use them as components.
<br/><br/>
We tried to keep the Agenda Template as flexible as possible, though there
must be many limitations, because it is generated dynamically.<br/><br/>
To keep the template flexible the following decisions were made:<br/>
1. Item tables.<br/>
1.a. there might be arbitrary number of Item tables.<br/>
1.b. Item tables design (bordewr, background) is arbitrary.<br/>
1.c. Items text styles are individual,
and use stylelist styles with predefined names.<br/>
As result the following limitations:<br/>
Pairs of Name->value for each item.<br/>
Tables contain *only* those pairs.<br/>
2. Topics table.<br/>
2.a. arbitrary structure.<br/>
2.b. design is arbitrary.<br/>
As result the following limitations:<br/>
No column merge is allowed.<br/>
One compolsary Heading row.<br/>
<br/><br/>
To let the template be flexible, we use a kind of "detection": we look where
the items are read the design of each table, reaplying it after writing the
table.AgendaTemplate.xTextDocument
<br/><br/>
A note about threads:<br/>
Many methods here are synchronized, in order to avoid colission made by
events fired too often.
@author rpiterman
'''
class AgendaTemplate(TextDocument):

    writtenTopics = []
    items = []
    itemsMap = {}
    document = None
    textSectionHandler = None
    template = None
    agenda = None
    lock = RLock()

    '''constructor. The document is *not* loaded here.
    only some formal members are set.
    @param  AgendaTemplate.document_ service factory.
    @param agenda_ the data model (CGAgenda)
    @param resources_ resources.
    '''

    def __init__(self,  xmsf, agenda, resources, listener):
        super(AgendaTemplate,self).__init__(xmsf,listener, None,
            "WIZARD_LIVE_PREVIEW")
        self.agenda = agenda
        self.resources = resources

    @synchronized(lock)
    def load(self, templateURL, topics):
        AgendaTemplate.template = self.calcTemplateName(templateURL)
        AgendaTemplate.xTextDocument = self.loadAsPreview(templateURL, False)
        self.xFrame.ComponentWindow.Enable = False
        self.xTextDocument.lockControllers()
        self.initialize()
        self.initializeData(topics)
        self.xTextDocument.unlockControllers()

    '''
    The agenda templates are in format of aw-XXX.ott
    the templates name is then XXX.ott.
    This method calculates it.
    @param url
    @return the template name without the "aw-" at the beginning.
    '''

    def calcTemplateName(self, url):
        return FileAccess.connectURLs(
            FileAccess.getParentDir(url), FileAccess.getFilename(url)[3:])

    '''synchronize the document to the model.<br/>
    this method rewrites all titles, item tables , and the topics table-
    thus synchronizing the document to the data model (CGAgenda).
    @param topicsData since the model does not contain Topics
    information (it is only actualized on save) the given list
    supplies this information.
    '''

    def initializeData(self, topicsData):
        for i in self.itemsTables:
            try:
                i.write("")
            except Exception, ex:
                traceback.print_exc()

        self.redrawTitle("txtTitle")
        self.redrawTitle("txtDate")
        self.redrawTitle("txtTime")
        self.redrawTitle("cbLocation")
        if self.agenda.cp_TemplateName is None:
            self.agenda.cp_TemplateName = ""
        self.setTemplateTitle(self.agenda.cp_TemplateName)

    '''redraws/rewrites the table which contains the given item
    This method is called when the user checks/unchecks an item.
    The table is being found, in which the item is, and redrawn.
    @param itemName
    '''

    @classmethod
    @synchronized(lock)
    def redraw(self, itemName):
        AgendaTemplate.xTextDocument.lockControllers()
        try:
            # get the table in which the item is...
            itemsTable = AgendaTemplate.itemsMap[itemName]
            # rewrite the table.
            itemsTable.write(None)
        except Exception, e:
            traceback.print_exc()
        AgendaTemplate.xTextDocument.unlockControllers()

    '''update the documents title property to the given title
    @param newTitle title.
    '''

    @synchronized(lock)
    def setTemplateTitle(self, newTitle):
        self.m_xDocProps.Title = newTitle

    '''checks the data model if the
    item corresponding to the given string should be shown
    '''
    def isShowItem(self, itemName):
        if itemName is "<meeting-type>":
            return self.agenda.cp_ShowMeetingType
        elif itemName is "<read>":
            return self.agenda.cp_ShowRead
        elif itemName is "<bring>":
            return self.agenda.cp_ShowBring
        elif itemName is "<notes>":
            return self.agenda.cp_ShowNotes
        elif itemName is "<facilitator>":
            return self.agenda.cp_ShowFacilitator
        elif itemName is "<timekeeper>":
            return self.agenda.cp_ShowTimekeeper
        elif itemName is "<notetaker>":
            return self.agenda.cp_ShowNotetaker
        elif itemName is "<attendees>":
            return self.agenda.cp_ShowAttendees
        elif itemName is "<called-by>":
            return self.agenda.cp_ShowCalledBy
        elif itemName is "<observers>":
            return self.agenda.cp_ShowObservers
        elif itemName is "<resource-persons>":
            return self.agenda.cp_ShowResourcePersons

    '''Initializes a template.<br/>
    This method does the following tasks:<br/>
    Get a Time and Date format for the document, and retrieve the null
    date of the document (which is document-specific).<br/>
    Initializes the Items Cache map.
    Analyses the document:<br/>
    -find all "fille-ins" (apear as &gt;xxx&lt; in the document).
    -analyze all items sections (and the tables in them).
    -locate the titles and actualize them
    -analyze the topics table
    '''

    def initialize(self):
        '''
        Get the default locale of the document,
        and create the date and time formatters.
        '''
        AgendaTemplate.dateUtils = Helper.DateUtils(
            self.xMSF, AgendaTemplate.xTextDocument)
        AgendaTemplate.formatter = AgendaTemplate.dateUtils.formatter
        AgendaTemplate.dateFormat = AgendaTemplate.dateUtils.getFormat(DATE_SYSTEM_LONG)
        AgendaTemplate.timeFormat = AgendaTemplate.dateUtils.getFormat(TIME_HHMM)

        '''
        get the document properties object.
        '''

        self.m_xDocProps = AgendaTemplate.xTextDocument.DocumentProperties
        self.addTitlesToPlaceHoldersDict()
        self.initializePlaceHolders()
        #self.initializeItemsSections()
        AgendaTemplate.textSectionHandler = TextSectionHandler(
            AgendaTemplate.xTextDocument, AgendaTemplate.xTextDocument)
        self.topics = Topics()


    def addTitlesToPlaceHoldersDict(self):
        #We add them here because we can't in AgendaWizardDialogResources.py
        self.resources.dictTemplatePlaceHolders["<minutes-title>"] = \
            self.agenda.cp_Title
        self.resources.dictTemplatePlaceHolders["<minutes-location>"] = \
            self.agenda.cp_Location
        self.resources.dictTemplatePlaceHolders["<minutes-date>"] = \
            self.agenda.cp_Date
        self.resources.dictTemplatePlaceHolders["<minutes-time>"] = \
            self.agenda.cp_Time
        
    def initializePlaceHolders(self):
        # Looks for all the placeholders in the template with the following
        # pattern: "<[^>]+>" and create a TextElement for each one of them   
        allItems = self.searchFillInItems(0)
        for i in xrange(allItems.Count):
            item = allItems.getByIndex(i)
            text = item.String.lower()
            aux = TextElement(
                item, self.resources.dictTemplatePlaceHolders[text],
                self.resources.resPlaceHolderHint, self.xMSF)
            if self.isShowItem(text):
                aux.write()
                

    '''
    analyze the item sections in the template.
    delegates the analyze of each table to the ItemsTable class.
    '''

    def initializeItemsSections(self):
        sections = self.getSections(AgendaTemplate.xTextDocument, SECTION_ITEMS)
        # for each section - there is a table...
        self.itemsTables = []
        for i in sections:
            try:
                self.itemsTables.append(
                    ItemsTable(self.getSection(i), self.getTable(i)))
            except Exception, ex:
                traceback.print_exc()
                raise AttributeError (
                    "Fatal Error while initialilzing" +
                    "Template: items table in section " + i)


    def getSections(self, document, s):
        allSections = document.TextSections.ElementNames
        return self.getNamesWhichStartWith(allSections, s)

    @classmethod
    def getSection(self, name):
        return getattr(AgendaTemplate.xTextDocument.TextSections, name)

    @classmethod
    def getTable(self, name):
        return getattr(AgendaTemplate.xTextDocument.TextTables, name)

    @classmethod
    @synchronized(lock)
    def redrawTitle(self, controlName):
        try:
            if controlName == "txtTitle":
                self.writeTitle(
                    AgendaTemplate.teTitle, AgendaTemplate.trTitle,
                    self.agenda.cp_Title)
            elif controlName == "txtDate":
                self.writeTitle(
                    AgendaTemplate.teDate, AgendaTemplate.trDate,
                    self.getDateString(self.agenda.cp_Date))
            elif controlName == "txtTime":
                self.writeTitle(
                    AgendaTemplate.teTime, AgendaTemplate.trTime,
                    self.getTimeString(self.agenda.cp_Time))
            elif controlName == "cbLocation":
                self.writeTitle(
                    AgendaTemplate.teLocation, AgendaTemplate.trLocation,
                    self.agenda.cp_Location)
            else:
                raise IllegalArgumentException ("No such title control...")
        except Exception:
            traceback.print_exc()

    @classmethod
    def writeTitle(self, te, tr, text):
        if text is None:
            te.text = ""
        else:
            te.text = text
        te.write(tr)

    @classmethod
    def getDateString(self, d):
        if d is None or d == "":
            return ""
        date = int(d)
        year = date / 10000
        month = (date % 10000) / 100
        day = date % 100
        dateObject = dateTimeObject(year, month, day)
        return AgendaTemplate.dateUtils.format(
            AgendaTemplate.dateFormat, dateObject)

    @classmethod
    def getTimeString(self, s):
        if s is None or s == "":
            return ""
        time = int(s)
        t = ((time / float(1000000)) / float(24)) \
            + ((time % 1000000) / float(1000000)) / float(35)
        return self.formatter.convertNumberToString(AgendaTemplate.timeFormat, t)

    @synchronized(lock)
    def finish(self, topics):
        self.createMinutes(topics)
        self.deleteHiddenSections()
        AgendaTemplate.textSectionHandler.removeAllTextSections()

    '''
    hidden sections exist when an item's section is hidden because the
    user specified not to display any items which it contains.
    When finishing the wizard removes this sections entireley from the document.
    '''

    def deleteHiddenSections(self):
        allSections = AgendaTemplate.xTextDocument.TextSections.ElementNames
        try:
            for i in allSections:
                self.section = self.getSection(i)
                visible = bool(Helper.getUnoPropertyValue(
                    self.section, "IsVisible"))
                if not visible:
                    self.section.Anchor.String = ""

        except Exception, ex:
            traceback.print_exc()

    '''
    create the minutes for the given topics or remove the minutes
    section from the document.
    If no topics are supplied, or the user specified not to create minuts,
    the minutes section will be removed,
    @param topicsData supplies PropertyValue arrays containing
    the values for the topics.
    '''

    @synchronized(lock)
    def createMinutes(self, topicsData):
        # if the minutes section should be removed (the
        # user did not check "create minutes")
        if not self.agenda.cp_IncludeMinutes \
                or len(topicsData) <= 1:
            try:
                minutesAllSection = self.getSection(SECTION_MINUTES_ALL)
                minutesAllSection.Anchor.String = ""
            except Exception, ex:
                traceback.print_exc()

        # the user checked "create minutes"
        else:
            try:
                '''
                now add minutes for each topic.
                The template contains *one* minutes section, so
                we first use the one available, and then add a one...
                topics data has *always* an empty topic at the end...
                '''

                for i in xrange(len(topicsData) - 1):
                    topic = topicsData[i]
                    AgendaTemplate.items = TextDocument.searchFillInItems()
                    itemIndex = 0
                    for item in self.items:
                            '''
                            if the topic has no time, we do not
                            display any time here.
                            '''
                            if topicTime == 0 or topicStartTime == 0:
                                time = topic[3].Value
                            else:
                                time = getTimeString(str(topicStartTime)) + " - "
                                topicStartTime += topicTime * 1000
                                time += getTimeString(str(topicStartTime))

                            fillMinutesItem(item, time, "")

                    AgendaTemplate.textSectionHandler.removeTextSectionbyName(
                        SECTION_MINUTES)
                    # after the last section we do not insert a one.
                    if i < len(topicsData) - 2:
                        AgendaTemplate.textSectionHandler.insertTextSection(
                            SECTION_MINUTES, AgendaTemplate.template, False)

            except Exception, ex:
                traceback.print_exc()

    def getNamesWhichStartWith(self, allNames, prefix):
        v = []
        for i in allNames:
            if i.startswith(prefix):
                v.append(i)
        return v

    '''Convenience method for inserting some cells into a table.
    @param table
    @param start
    @param count
    '''

    @classmethod
    def insertTableRows(self, table, start, count):
        rows = table.Rows
        rows.insertByIndex(start, count)

    '''returns the row index for this cell name.
    @param cellName
    @return the row index for this cell name.
    '''

    @classmethod
    def getRowIndex(self, cellName):
        return int(cellName.RangeName[1:])

    '''returns the rows count of this table, assuming
    there is no vertical merged cells.
    @param table
    @return the rows count of the given table.
    '''

    @classmethod
    def getRowCount(self, table):
        cells = table.getCellNames()
        return int(cells[len(cells) - 1][1:])

class ItemsTable(object):
    '''
    the items in the table.
    '''
    items = []
    table = None

    def __init__(self, section_, table_):
        ItemsTable.table = table_
        self.section = section_
        self.items = []

    '''
    link the section to the template. this will restore the original table
    with all the items.<br/>
    then break the link, to make the section editable.<br/>
    then, starting at cell one, write all items that should be visible.
    then clear the rest and remove obsolete rows.
    If no items are visible, hide the section.
    @param dummy we need a param to make this an Implementation
    of AgendaElement.
    @throws Exception
    '''

    def write(self, dummy):
        with AgendaTemplate.lock:
            name = self.section.Name
            # link and unlink the section to the template.
            AgendaTemplate.textSectionHandler.linkSectiontoTemplate(
                AgendaTemplate.template, name, self.section)
            AgendaTemplate.textSectionHandler.breakLinkOfTextSection(
                self.section)
            # we need to get a instance after linking.
            ItemsTable.table = AgendaTemplate.getTable(name)
            self.section = AgendaTemplate.getSection(name)
            cursor = ItemsTable.table.createCursorByCellName("A1")
            # should this section be visible?
            visible = False
            # write items
            # ===========
            cellName = ""
            '''
            now go through all items that belong to this
            table. Check each one agains the model. If it should
            be display, call it's write method.
            All items are of type AgendaItem which means they write
            two cells to the table: a title (text) and a placeholder.
            see AgendaItem class below.
            '''
            for i in self.items:
                if AgendaTemplate.isShowItem(i.name):
                    visible = True
                    i.table = ItemsTable.table
                    i.write(cursor)
                    # I store the cell name which was last written...
                    cellName = cursor.RangeName
                    cursor.goRight(1, False)

            if visible:
                boolean = True
            else:
                boolean = False
            Helper.setUnoPropertyValue(self.section, "IsVisible", boolean)
            if not visible:
                return
                '''
                remove obsolete rows
                ====================
                if the cell that was last written is the current cell,
                it means this is the end of the table, so we end here.
                (because after getting the cellName above,
                I call the goRight method.
                If it did not go right, it means its the last cell.
                '''

            if cellName == cursor.RangeName:
                return
                '''
                if not, we continue and clear all cells until
                we are at the end of the row.
                '''

            while not cellName == cursor.RangeName and \
                    not cursor.RangeName.startswith("A"):
                cell = ItemsTable.table.getCellByName(cursor.RangeName)
                cell.String = ""
                cellName = cursor.RangeName
                cursor.goRight(1, False)

            '''
            again: if we are at the end of the table, end here.
            '''
            if cellName == cursor.RangeName:
                return

            rowIndex = AgendaTemplate.getRowIndex(cursor)
            rowsCount = AgendaTemplate.getRowCount(ItemsTable.table)
            '''
            now before deleteing i move the cursor up so it
            does not disappear, because it will crash office.
            '''
            cursor.gotoStart(False)

'''
This class handles the preview of the topics table.
You can call it the controller of the topics table.
It differs from ItemsTable in that it has no data model -
the update is done programttically.<br/>
<br/>
The decision to make this class a class by its own
was done out of logic reasons and not design/functionality reasons,
since there is anyway only one instance of this class at runtime
it could have also be implemented in the AgendaTemplate class
but for clarity and separation I decided to make a sub class for it.

@author rp143992
'''

class Topics(object):
    '''Analyze the structure of the Topics table.
    The structure Must be as follows:<br>
    -One Header Row. <br>
    -arbitrary number of rows per topic <br>
    -arbitrary content in the topics row <br>
    -only soft formatting will be restored. <br>
    -the topic rows must repeat three times. <br>
    -in the topics rows, placeholders for number, topic, responsible,
    and duration must be placed.<br><br>
    A word about table format: to reconstruct the format of the table we hold
    to the following formats: first row (header), topic, and last row.
    We hold the format of the last row, because one might wish to give it
    a special format, other than the one on the bottom of each topic.
    The left and right borders of the whole table are, on the other side,
    part of the topics rows format, and need not be preserved seperateley.
    '''
    table = None
    lastRowFormat = []
    numCell = -1
    topicCell = -1
    responsibleCell = -1
    timeCell = -1
    rowsPerTopic = None
    topicCells = []

    def __init__(self):
        self.topicItems = {}
        self.firstRowFormat = []
        # This is the topics table. say hallo :-)
        try:
            Topics.table = AgendaTemplate.getTable(SECTION_TOPICS)
        except Exception, ex:
            traceback.print_exc()
            raise AttributeError (
                "Fatal error while loading template: table " + \
                SECTION_TOPICS + " could not load.")

        '''
        in the topics table, there are always one
        title row and three topics defined.
        So no mutter how many rows a topic takes - we
        can restore its structure and format.
        '''
        rows = AgendaTemplate.getRowCount(Topics.table)
        Topics.rowsPerTopic = (rows - 1) / 3
        firstCell = "A" + str(1 + Topics.rowsPerTopic + 1)
        afterLastCell = "A" + str(1 + (Topics.rowsPerTopic * 2) + 1)
        # go to the first row of the 2. topic
        cursor = Topics.table.createCursorByCellName(firstCell)
        # analyze the structure of the topic rows.
        while not cursor.RangeName == afterLastCell:
            cell = Topics.table.getCellByName(cursor.RangeName)
            # if the cell contains a relevant <...>
            # i add the text element to the hash,
            # so it's text can be updated later.
            try:
                if items[cell.CellName] is not None:
                    self.topicItems[cell.String.lower().lstrip()] = cell
            except KeyError:
                pass

            Topics.topicCells.append(cell)
            # goto next cell.
            cursor.goRight(1, False)
        '''
        now - in which cell is every fillin?
        '''

        Topics.numCell = Topics.topicCells.index(
            self.topicItems[FILLIN_TOPIC_NUMBER])
        Topics.topicCell = Topics.topicCells.index(
            self.topicItems[FILLIN_TOPIC_TOPIC])
        Topics.responsibleCell = Topics.topicCells.index(
            self.topicItems[FILLIN_TOPIC_RESPONSIBLE])
        Topics.timeCell = Topics.topicCells.index(
            self.topicItems[FILLIN_TOPIC_TIME])

    '''@param topic the topic number to write
    @param data the data of the topic.
    @return the number of rows that have been added
    to the table. 0 or a negative number: no rows added.
    '''

    @classmethod
    def write2(self, topic, data):
        if topic >= len(AgendaTemplate.writtenTopics):
            size = topic - len(AgendaTemplate.writtenTopics)
            AgendaTemplate.writtenTopics += [None] * size
        AgendaTemplate.writtenTopics.insert(topic, "")
        # make sure threr are enough rows for me...
        rows = AgendaTemplate.getRowCount(Topics.table)
        reqRows = 1 + (topic + 1) * Topics.rowsPerTopic
        firstRow = reqRows - Topics.rowsPerTopic + 1
        diff = reqRows - rows
        if diff > 0:
            AgendaTemplate.insertTableRows(Topics.table, rows, diff)
            # set the item's text...

        self.setItemText(Topics.numCell, data[0].Value)
        self.setItemText(Topics.topicCell, data[1].Value)
        self.setItemText(Topics.responsibleCell, data[2].Value)
        self.setItemText(Topics.timeCell, data[3].Value)
        # now write !
        cursor = Topics.table.createCursorByCellName("A" + str(firstRow))
        for i in Topics.topicCells:
            i.write(Topics.table.getCellByName(cursor.RangeName))
            cursor.goRight(1, False)
        # now format !
        cursor.gotoCellByName("A" + str(firstRow), False)
        return diff

    '''check if the topic with the given index is written to the table.
    @param topic the topic number (0 base)
    @return true if the topic is already written to the table. False if not.
    (false would mean rows must be added to the table in order to
    be able to write this topic).
    '''

    def isWritten(self, topic):
        return (len(AgendaTemplate.writtenTopics) > topic \
            and AgendaTemplate.writtenTopics[topic] is not None)

    '''rewrites a single cell containing.
    This is used in order to refresh the topic/responsible/duration data
    in the preview document, in response to a change in the gui (by the user)
    Since the structure of the topics table is flexible,
    we don't reference a cell number. Rather, we use "what" argument to
    specify which cell should be redrawn.
    The Topics object, which analyzed the structure of the topics table appon
    initialization, refreshes the approperiate cell.
    @param topic index of the topic (0 based).
    @param what 0 for num, 1 for topic, 2 for responsible, 3 for duration
    @param data the row's data.
    @throws Exception if something goes wrong (thow nothing should)
    '''

    def writeCell(self, topic, what, data):
        # if the whole row should be written...
        if not self.isWritten(topic):
            self.write(topic, data)
            # write only the "what" cell.
        else:
            # calculate the table row.
            firstRow = 1 + (topic * Topics.rowsPerTopic) + 1
            # go to the first cell of this topic.
            cursor = Topics.table.createCursorByCellName("A" + str(firstRow))
            te = None
            cursorMoves = 0
            if what == 0:
                te = self.setItemText(Topics.numCell, data[0].Value)
                cursorMoves = Topics.numCell
            elif what == 1:
                te = self.setItemText(Topics.topicCell, data[1].Value)
                cursorMoves = Topics.topicCell
            elif what == 2:
                te = self.setItemText(Topics.responsibleCell, data[2].Value)
                cursorMoves = Topics.responsibleCell
            elif what == 3:
                te = self.setItemText(Topics.timeCell, data[3].Value)
                cursorMoves = Topics.timeCell

            # move the cursor to the needed cell...
            cursor.goRight(cursorMoves, False)
            xc = Topics.table.getCellByName(cursor.RangeName)
            # and write it !
            te.write(xc)

    '''writes the given topic.
    if the first topic was involved, reformat the
    first row.
    If any rows were added to the table, reformat
    the last row.
    @param topic the index of the topic to write.
    @param data the topic's data. (see TopicsControl
    for explanation about the topics data model)
    @throws Exception if something goes wrong (though nothing should).
    '''

    def write(self, topic, data):
        diff = self.write2(topic, data)
        '''if the first topic has been written,
        one needs to reformat the first row.
        '''
        if topic == 0:
            self.formatFirstRow()
        '''
        if any rows were added, one needs to format
        the whole table again.
        '''

        if diff > 0:
            self.formatLastRow()

    '''removes obsolete rows, reducing the
    topics table to the given number of topics.
    Note this method does only reducing - if
    the number of topics given is greater than the
    number of actuall topics it does *not* add
    rows !
    Note also that the first topic will never be removed.
    If the table contains no topics, the whole section will
    be removed uppon finishing.
    The reason for that is a "table-design" one: the first topic is
    maintained in order to be able to add rows with a design of this topic,
    and not of the header row.
    @param topics the number of topics the table should contain.
    @throws Exception
    '''

    def reduceDocumentTo(self, topics):
        # we never remove the first topic...
        if topics <= 0:
            topics = 1

        tableRows = Topics.table.Rows
        targetNumOfRows = topics * Topics.rowsPerTopic + 1
        '''if tableRows.Count > targetNumOfRows:
            tableRows.removeByIndex(
                targetNumOfRows, tableRows.Count - targetNumOfRows)'''

        self.formatLastRow()
        while len(AgendaTemplate.writtenTopics) > topics:
            del AgendaTemplate.writtenTopics[topics]

    '''reapply the format of the first (header) row.
    '''

    def formatFirstRow(self):
        cursor = Topics.table.createCursorByCellName("A1")
        self.formatTable(cursor, self.firstRowFormat, False)

    '''reaply the format of the last row.
    '''
    @classmethod
    def formatLastRow(self):
        cursor = Topics.table.createCursorByCellName("A1")
        cursor.gotoEnd(False)
        self.formatTable(cursor, Topics.lastRowFormat, True)

    '''returns a text element for the given cell,
    which will write the given text.
    @param cell the topics cell number.
    @param value the value to write.
    @return a TextElement object which will write the given value
    to the given cell.
    '''

    @classmethod
    def setItemText(self, cell, value):
        if cell >= 0:
            te = Topics.topicCells[cell]
            if te is not None:
                te.text = str(value)
            return te

        return None

    '''formats a series of cells from the given one,
    using the given List of TableCellFormatter objects,
    in the given order.
    This method is used to format the first (header) and the last
    rows of the table.
    @param cursor a table cursor, pointing to the start cell to format
    @param formats a List containing TableCellFormatter objects.
    Each will format one cell in the direction specified.
    @param reverse if true the cursor will move left,
    formatting in reverse order (used for the last row).
    '''
    @classmethod
    def formatTable(self, cursor, formats, reverse):
        for i in formats:
            i.format(Topics.table.getCellByName(cursor.RangeName))
            if reverse:
                cursor.goLeft(1, False)
            else:
                cursor.goRight(1, False)

'''
An implementation of AgendaElement which
gets as a parameter a table cursor, and writes
a text to the cell marked by this table cursor, and
a place holder to the next cell.
@author rp143992
'''

class AgendaItem(object):

    def __init__(self, name, textRange, field):
        self.name = name
        self.field = field
        self.textRange = textRange

    def write(self, tableCursor):
        cellname = tableCursor.RangeName
        cell = ItemsTable.table.getCellByName(cellname)
        self.textElement.String = cell
        tableCursor.goRight(1, False)
        #second field is actually always null...
        # this is a preparation for adding placeholders.
        if self.field is not None:
            self.field.write(ItemsTable.table.getCellByName(
                tableCursor.RangeName))
