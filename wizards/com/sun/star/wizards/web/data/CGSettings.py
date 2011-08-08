import traceback
from common.FileAccess import FileAccess
from common.ConfigGroup import ConfigGroup
from common.ConfigSet import ConfigSet
from CGExporter import CGExporter
from CGLayout import CGLayout
from CGStyle import CGStyle
from CGIconSet import CGIconSet
from CGImage import CGImage
from CGFilter import CGFilter
from common.Helper import Helper
from CGSessionName import CGSessionName
from CGSession import CGSession
from common.Properties import Properties

from com.sun.star.i18n.NumberFormatIndex import DATE_SYS_DMMMYYYY
from com.sun.star.i18n.NumberFormatIndex import NUMBER_1000DEC2

class CGSettings(ConfigGroup):

    RESOURCE_PAGES_TEMPLATE = 0
    RESOURCE_SLIDES_TEMPLATE = 1
    RESOURCE_CREATED_TEMPLATE = 2
    RESOURCE_UPDATED_TEMPLATE = 3
    RESOURCE_SIZE_TEMPLATE = 4

    cp_WorkDir = str()
    cp_Exporters = ConfigSet(CGExporter())
    cp_Layouts = ConfigSet(CGLayout())
    cp_Styles = ConfigSet(CGStyle())
    cp_IconSets = ConfigSet(CGIconSet())
    cp_BackgroundImages = ConfigSet(CGImage())
    cp_SavedSessions = ConfigSet(CGSessionName())
    cp_Filters = ConfigSet(CGFilter())
    savedSessions = ConfigSet(CGSessionName())
    cp_DefaultSession = CGSession()
    cp_LastSavedSession = str()

    def __init__(self, xmsf_, resources_, document):
        self.xmsf = xmsf_
        try:
            self.soTemplateDir = FileAccess.getOfficePath2(
                self.xmsf, "Config", "", "")
            self.soGalleryDir = FileAccess.getOfficePath2(
                self.xmsf, "Gallery", "share", "")
            root = self
            self.formatter = self.Formatter(self.xmsf, document)
            self.resources = resources_
            self.workPath = None
            self.exportersMap = {}
        except Exception:
            traceback.print_exc()

    def getExporters(self, mime):
        exps = self.exportersMap.get(mime)
        if exps is None:
            self.exportersMap.put(mime, exps = createExporters(mime))

        return exps

    def createExporters(self, mime):
        exporters = self.cp_Exporters.items()
        v = Vector.Vector()
        i = 0
        while i < exporters.length:
            if (exporters[i]).supports(mime):
                try:
                    v.add(exporters[i])
                except Exception, ex:
                    ex.printStackTrace()

            i += 1
        return v.toArray(self.__class__.EMPTY_ARRAY_1)

    '''
    call after read.
    @param xmsf
    @param document the background document. used for date/number formatting.
    '''

    def configure(self, xmsf):
        self.workPath = FileAccess.connectURLs(
            self.soTemplateDir, self.cp_WorkDir)
        #COMMENTED
        #self.calcExportersTargetTypeNames(xmsf)

    def calcExportersTargetTypeNames(self, xmsf):
        typeDetect = xmsf.createInstance(
            "com.sun.star.document.TypeDetection")
        for i in xrange(self.cp_Exporters.getSize()):
            self.calcExporterTargetTypeName(
                typeDetect, self.cp_Exporters.getElementAt(i))

    def calcExporterTargetTypeName(self, typeDetect, exporter):
        if not exporter.cp_TargetType == "":
            exporter.targetTypeName = Properties.getPropertyValue(
                typeDetect.getByName(exporter.cp_TargetType), "UIName")

    @classmethod
    def getFileAccess(self, xmsf = None):
        if xmsf is None:
            xmsf = self.xmsf
        if self.fileAccess is None:
            self.fileAccess = FileAccess.FileAccess_unknown(xmsf)

        return self.fileAccess

    class Formatter(object):
        def __init__(self, xmsf, document):
            self.dateUtils = Helper.DateUtils(xmsf, document)
            self.dateFormat = self.dateUtils.getFormat(DATE_SYS_DMMMYYYY)
            self.numberFormat = self.dateUtils.getFormat(NUMBER_1000DEC2)

        def formatCreated(self, date):
            sDate = self.dateUtils.format(dateFormat, date)
            return resources[CGSettings.RESOURCE_CREATED_TEMPLATE].replace(
                "%DATE", sDate)

        def formatUpdated(self, date):
            sDate = self.dateUtils.format(dateFormat, date);
            return resources[CGSettings.RESOURCE_UPDATED_TEMPLATE].replace(
                "%DATE", sDate)

        def formatFileSize(self, size):
            sizeInKB = size / float(1024)
            sSize = self.dateUtils.getFormatter().convertNumberToString(
                numberFormat, sizeInKB)
            return resources[CGSettings.RESOURCE_SIZE_TEMPLATE].replace(
                "%NUMBER", sSize)
