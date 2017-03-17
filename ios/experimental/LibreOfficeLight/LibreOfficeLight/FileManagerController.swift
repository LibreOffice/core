//
//ThisfileispartoftheLibreOfficeproject.
//
//ThisSourceCodeFormissubjecttothetermsoftheMozillaPublic
//License,v.2.0.IfacopyoftheMPLwasnotdistributedwiththis
//file,Youcanobtainoneathttp://mozilla.org/MPL/2.0/.
//
importUIKit


privateclassFileStorage
{
//housekeepingvariables
privateletfilemgr:FileManager=FileManager.default
privatevarstorageIsLocal:Bool=true

//Startpathforthe2storagelocations
privateletbaseLocalDocPath:URL
privateletbaseCloudDocPath:URL?
privatevarcurrentDocPath:URL?{
get{
returnstorageIsLocal?baseLocalDocPath:baseCloudDocPath
}
}

//makeaccesstocurrentdirindependentofstorageselection
privatevarlocalDir:URL
privatevarcloudDir:URL?
privatevarcurrentDir:URL{
get{
returnstorageIsLocal?localDir:cloudDir!
}
set(newDir){
ifstorageIsLocal{
localDir=newDir
}else{
cloudDir=newDir
}
}
}



//contentofcurrentdirectory
varcurrentFileList:[String]=[]
varcurrentDirList:[String]=[]



//Supportfunctions
funciCloudEnabled()->Bool
{
returnfilemgr.ubiquityIdentityToken!=nil
}


funcisSubDirectory()->Bool
{
returncurrentDir!=currentDocPath
}



funcselectStorage(_doSwitch:Bool)->Bool
{
ifdoSwitch{
storageIsLocal=!storageIsLocal
buildFileList()
}
returnstorageIsLocal
}



funcenterDirectory(_name:String)
{
//simpleadddirectory
currentDir=currentDir.appendingPathComponent(name)
filemgr.changeCurrentDirectoryPath(name)
buildFileList()
}


funcleaveDirectory()
{
//stepupforactivestorage,andonlyifnotinroot
ifisSubDirectory(){
currentDir=currentDir.deletingLastPathComponent()
buildFileList()
}
}



funccreateDirectory(_name:String)
{
letnewDir=currentDir.appendingPathComponent(name)
try!filemgr.createDirectory(at:newDir,withIntermediateDirectories:true,attributes:nil)
currentDir=currentDir.appendingPathComponent(name)
buildFileList()
}



funcdeleteFileDirectory(_name:String)
{
letdelDir=currentDir.appendingPathComponent(name)
try!filemgr.removeItem(at:delDir)
buildFileList()
}



funcgetFileURL(_name:String)->URL
{
returncurrentDir.appendingPathComponent(name)
}



funccopyFile(_name:String)
{
try!filemgr.copyItem(at:currentDir.appendingPathComponent(name),
to:(storageIsLocal?cloudDir!:localDir).appendingPathComponent(name))
}



funcmoveFile(_name:String)
{
try!filemgr.moveItem(at:currentDir.appendingPathComponent(name),
to:(storageIsLocal?localDir:cloudDir!).appendingPathComponent(name))
buildFileList()
}



funcrenameFile(_oldName:String,_newName:String)
{
try!filemgr.moveItem(at:currentDir.appendingPathComponent(oldName),
to:currentDir.appendingPathComponent(newName))
buildFileList()
}



privatefuncbuildFileList()
{
currentDirList=[]
currentFileList=[]
letrawFileList=try!filemgr.contentsOfDirectory(at:currentDir,
includingPropertiesForKeys:[URLResourceKey.isDirectoryKey])
forrawFileinrawFileList{
varisDir:ObjCBool=false
filemgr.fileExists(atPath:rawFile.path,isDirectory:&isDir)
ifisDir.boolValue{
currentDirList.append(rawFile.lastPathComponent)
}else{
currentFileList.append(rawFile.lastPathComponent)
}
}
}



init()
{
baseLocalDocPath=filemgr.urls(for:.documentDirectory,in:.userDomainMask)[0]
localDir=baseLocalDocPath

letcloudUrl=filemgr.url(forUbiquityContainerIdentifier:nil)
baseCloudDocPath=(cloudUrl==nil)?nil:cloudUrl?.appendingPathComponent("Documents")
cloudDir=baseCloudDocPath
buildFileList()
}
}



classFileManagerController:UITableViewController,actionsControlDelegate

{
//Housekeepingvariables
privatevarfileData=FileStorage()
privatevarselectedRow:IndexPath?



//selectStorageisonlyenabledwheniCloudisactive
@IBOutletweakvarbuttonSelectStorage:UIBarButtonItem!
overridefuncviewDidLoad()
{
super.viewDidLoad()
buttonSelectStorage.isEnabled=fileData.iCloudEnabled()
}



//Tooglebetweenlocalandcloudstorage
@IBActionfuncdoSelectStorage(_sender:UIBarButtonItem)
{
sender.image=fileData.selectStorage(true)?#imageLiteral(resourceName:"iCloudDrive"):#imageLiteral(resourceName:"iPhone")
reloadData()
self.presentedViewController?.dismiss(animated:true,completion:nil)
}



//Laststopbeforedisplayingpopover
overridefuncprepare(forsegue:UIStoryboardSegue,sender:Any?)
{
ifsegue.identifier=="showActions"{
letvc=segue.destinationas!FileManagerActions
vc.delegate=self
vc.inFileSelect=(selectedRow!=nil)
vc.inSubDirectory=fileData.isSubDirectory()
vc.useCloud=fileData.iCloudEnabled()
}
}



funcactionOpen()
{
ifselectedRow!=nil{
letcurrentCell=tableView.cellForRow(at:selectedRow!)as!FileManagerCell
ifcurrentCell.isDirectory{
fileData.enterDirectory(currentCell.fileName)
reloadData()
}else{
//JIXdelegatetoDocument
}
}
}



funcactionDelete()
{
ifselectedRow!=nil{
letcurrentCell=self.tableView.cellForRow(at:selectedRow!)as!FileManagerCell
fileData.deleteFileDirectory(currentCell.fileName)
reloadData()
}
}



funcactionRename(_name:String)
{
ifselectedRow!=nil{
letcurrentCell=tableView.cellForRow(at:selectedRow!)as!FileManagerCell
fileData.renameFile(currentCell.fileName,name)
reloadData()
}
}



funcactionUploadDownload()
{
ifselectedRow!=nil{
letcurrentCell=self.tableView.cellForRow(at:selectedRow!)as!FileManagerCell
fileData.copyFile(currentCell.fileName)
reloadData()
}
}



funcactionLevelUp()
{
fileData.leaveDirectory()
reloadData()
}



funcactionCreateDirectory(_name:String)
{
fileData.createDirectory(name)
reloadData()
}



//Tablehandlingfunctions
overridefuncnumberOfSections(intableView:UITableView)->Int
{
return1
}



overridefunctableView(_tableView:UITableView,numberOfRowsInSectionsection:Int)->Int
{
returnfileData.currentDirList.count+fileData.currentFileList.count
}



overridefunctableView(_tableView:UITableView,cellForRowAtindexPath:IndexPath)->UITableViewCell
{
letcell=self.tableView.dequeueReusableCell(withIdentifier:"fileEntry",for:indexPath)as!FileManagerCell
letrow=indexPath.row

ifrow<fileData.currentDirList.count{
cell.fileName=fileData.currentDirList[row]
cell.fileLabel.text=cell.fileName+"/"
cell.isDirectory=true
}else{
letinx=row-fileData.currentDirList.count
cell.fileName=fileData.currentFileList[inx]
cell.fileLabel.text=cell.fileName
cell.isDirectory=false
}
returncell
}



//Selectarow(file)andshowactions
overridefunctableView(_tableView:UITableView,didSelectRowAtindexPath:IndexPath)
{
selectedRow=indexPath
performSegue(withIdentifier:"showActions",sender:self)
}



//Supportfunction
funcreloadData()
{
selectedRow=nil
tableView.reloadData()
}

}



//Spaceholderforextrainformationneededtodotherightthingforeachaction
classFileManagerCell:UITableViewCell{

@IBOutletweakvarfileLabel:UILabel!
varisDirectory:Bool=false
varfileName:String=""
}



//Protocolforactionpopovercallback
protocolactionsControlDelegate
{
funcactionOpen()
funcactionDelete()
funcactionRename(_name:String)
funcactionUploadDownload()
funcactionLevelUp()
funcactionCreateDirectory(_name:String)
}



//Actionpopoverdialog
classFileManagerActions:UITableViewController

{
//Pointertocallbackclass
vardelegate:actionsControlDelegate?
varinSubDirectory:Bool=false
varinFileSelect:Bool=false
varuseCloud:Bool=false

//Callingclassmightenable/disableeachbutton
@IBOutletweakvarbuttonUploadDownload:UIButton!
@IBOutletweakvarbuttonDelete:UIButton!
@IBOutletweakvarbuttonOpen:UIButton!
@IBOutletweakvarbuttonRename:UIButton!
@IBOutletweakvarbuttonLevelUp:UIButton!


//Actions
@IBActionfuncdoOpen(_sender:UIButton)
{
delegate?.actionOpen()
dismiss(animated:false)
}



@IBActionfuncdoDelete(_sender:UIButton)
{
delegate?.actionDelete()
dismiss(animated:false)
}



@IBActionfuncdoUploadDownload(_sender:UIButton)
{
delegate?.actionUploadDownload()
dismiss(animated:false)
}



@IBActionfuncdoLevelUp(_sender:UIButton)
{
delegate?.actionLevelUp()
dismiss(animated:false)
}



overridefuncviewDidLoad()
{
super.viewDidLoad()
buttonLevelUp.isEnabled=inSubDirectory
buttonDelete.isEnabled=inFileSelect
buttonOpen.isEnabled=inFileSelect
buttonRename.isEnabled=inFileSelect
buttonUploadDownload.isEnabled=(inFileSelect&&useCloud)
}



//Laststopbeforedisplayingpopover
overridefuncprepare(forsegue:UIStoryboardSegue,sender:Any?)
{
letvc=segue.destinationas!setNameAction
vc.delegate=self.delegate
vc.protocolActionToPerform=(segue.identifier=="showRename")?0:1
}
}



//Actionpopoverdialog
classsetNameAction:UIViewController

{
//Pointertocallbackclass
vardelegate:actionsControlDelegate?
varprotocolActionToPerform:Int=-1


//Callingclassmightenable/disableeachbutton
@IBOutletweakvareditText:UITextField!



@IBActionfuncdoOK(_sender:UIButton)
{
print("checking\(protocolActionToPerform)")
switchprotocolActionToPerform
{
case0:
print("runrenameDir")
delegate?.actionRename(editText.text!)
case1:
print("runcreateDir")
delegate?.actionCreateDirectory(editText.text!)
default:
break
}
dismiss(animated:false)
}



overridefuncviewDidLoad()
{
super.viewDidLoad()
}
}

