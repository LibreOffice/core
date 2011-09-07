from common.ConfigGroup import ConfigGroup

'''
A Class which describes the publishing arguments
in a session.
Each session can contain different publishers, which are configured
through such a CGPublish object.
'''

class CGPublish(ConfigGroup):

    cp_Publish = bool()
    cp_URL = str()
    cp_Username = str()
    password = str()

    def setURL(self, path):
        try:
            self.cp_URL = (self.root).getFileAccess().getURL(path)
            self.overwriteApproved = False
        except Exception, ex:
            ex.printStackTrace()

    def getURL(self):
        try:
            return (self.root).getFileAccess().getPath(self.cp_URL, None)
        except Exception, e:
            e.printStackTrace()
            return ""

    def ftpURL(self):
        pass
        #COMMENTED
        #return "ftp://" + self.cp_Username + ((self.password != None and self.password.length() > 0) ? ":" + self.password : "") + "@" + self.cp_URL.substring(7)
