//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v.2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit


private class FileStorage
{
    // house keeping variables
    private let filemgr          : FileManager = FileManager.default
    private var storageIsLocal   : Bool = true

    // Start path for the 2 storage locations
    private let baseLocalDocPath : URL
    private let baseCloudDocPath : URL?
    private var currentDocPath  : URL? {
        get {
            return storageIsLocal ? baseLocalDocPath : baseCloudDocPath
        }
    }

    // make access to current dir independent of storage selection
    private var localDir  : URL
    private var cloudDir  : URL?
    private var currentDir : URL {
        get {
            return storageIsLocal ? localDir : cloudDir!
        }
        set(newDir) {
            if storageIsLocal {
               localDir = newDir
            } else {
               cloudDir = newDir
            }
        }
    }



    // content of current directory
    var currentFileList : [String] = []
    var currentDirList  : [String] = []



    // Support functions
    func iCloudEnabled() -> Bool
    {
        return filemgr.ubiquityIdentityToken != nil
    }


    func isSubDirectory() -> Bool
    {
        return currentDir != currentDocPath
    }



    func selectStorage(_ doSwitch : Bool) -> Bool
    {
        if doSwitch {
            storageIsLocal = !storageIsLocal
            buildFileList()
        }
        return storageIsLocal
    }



    func enterDirectory(_ name: String)
    {
        // simple add directory
        currentDir = currentDir.appendingPathComponent(name)
        filemgr.changeCurrentDirectoryPath(name)
        buildFileList()
    }


    func leaveDirectory()
    {
        // step up for active storage, and only if not in root
        if isSubDirectory() {
            currentDir = currentDir.deletingLastPathComponent()
            buildFileList()
        }
    }



    func getDirectory() -> String
    {
        let myCurDir = currentDir.path
        let myLen    = currentDocPath!.path.endIndex
        return (myLen == myCurDir.endIndex) ? "/" : myCurDir.substring(from: myLen)
    }



    func createDirectory(_ name: String)
    {
        let newDir = currentDir.appendingPathComponent(name)
        try! filemgr.createDirectory(at: newDir, withIntermediateDirectories: true, attributes: nil)
        currentDir = currentDir.appendingPathComponent(name)
        buildFileList()
    }



    func deleteFileDirectory(_ name: String)
    {
        let delDir = currentDir.appendingPathComponent(name)
        try! filemgr.removeItem(at: delDir)
        buildFileList()
    }



    func getFileURL(_ name: String) -> URL
    {
        return currentDir.appendingPathComponent(name)
    }



    func copyFile(_ name: String)
    {
        try! filemgr.copyItem(at: currentDir.appendingPathComponent(name),
                              to: (storageIsLocal ? cloudDir! : localDir).appendingPathComponent(name))
    }



    func moveFile(_ name: String)
    {
        try! filemgr.moveItem(at: currentDir.appendingPathComponent(name),
                              to: (storageIsLocal ? localDir : cloudDir!).appendingPathComponent(name))
        buildFileList()
    }



    func renameFile(_ oldName: String, _ newName: String)
    {
        try! filemgr.moveItem(at: currentDir.appendingPathComponent(oldName),
                              to: currentDir.appendingPathComponent(newName))
        buildFileList()
    }



    private func buildFileList()
    {
        currentDirList = []
        currentFileList = []
        let rawFileList = try! filemgr.contentsOfDirectory(at: currentDir,
                                                           includingPropertiesForKeys: [URLResourceKey.isDirectoryKey])
        for rawFile in rawFileList {
            var isDir: ObjCBool = false
            filemgr.fileExists(atPath: rawFile.path, isDirectory: &isDir)
            if isDir.boolValue {
                currentDirList.append(rawFile.lastPathComponent)
            } else {
                currentFileList.append(rawFile.lastPathComponent)
            }
        }
    }



    init()
    {
        baseLocalDocPath = filemgr.urls(for: .documentDirectory, in: .userDomainMask)[0]
        localDir         = baseLocalDocPath

        let cloudUrl     = filemgr.url(forUbiquityContainerIdentifier: nil)
        baseCloudDocPath = (cloudUrl == nil) ? nil : cloudUrl?.appendingPathComponent("Documents")
        cloudDir         = baseCloudDocPath
        buildFileList()
    }
}



class FileManagerController : UITableViewController, FileActionsControlDelegate

{
    // Housekeeping variables
    private var fileData = FileStorage()
    private var selectedRow : IndexPath?



    // selectStorage is only enabled when iCloud is active
    @IBOutlet weak var buttonSelectStorage: UIBarButtonItem!
    override func viewDidLoad()
    {
        super.viewDidLoad()
        buttonSelectStorage.isEnabled = fileData.iCloudEnabled()
    }



    // Toogle between local and cloud storage
    @IBAction func doSelectStorage(_ sender: UIBarButtonItem)
    {
        sender.image = fileData.selectStorage(true) ? #imageLiteral(resourceName: "iCloudDrive") : #imageLiteral(resourceName: "iPhone")
        reloadData()
        self.presentedViewController?.dismiss(animated: true, completion: nil)
    }



    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        if segue.identifier == "showActions" {
            let vc = segue.destination as! FileManagerActions
            vc.delegate = self
            vc.inFileSelect = (selectedRow != nil)
            vc.inSubDirectory = fileData.isSubDirectory()
            vc.useCloud = fileData.iCloudEnabled()
        }
    }



    func actionOpen()
    {
        if selectedRow != nil {
            let currentCell = tableView.cellForRow(at: selectedRow!) as! FileManagerCell
            if currentCell.isDirectory {
                fileData.enterDirectory(currentCell.fileName)
                reloadData()
            } else {
                // JIX delegate to Document
            }
        }
    }



    func actionDelete()
    {
        if selectedRow != nil {
            let currentCell = self.tableView.cellForRow(at: selectedRow!) as! FileManagerCell
            fileData.deleteFileDirectory(currentCell.fileName)
            reloadData()
        }
    }



    func actionRename(_ name : String)
    {
        if selectedRow != nil {
            let currentCell = tableView.cellForRow(at: selectedRow!) as! FileManagerCell
            fileData.renameFile(currentCell.fileName, name)
            reloadData()
        }
    }



    func actionUploadDownload()
    {
        if selectedRow != nil {
            let currentCell = self.tableView.cellForRow(at: selectedRow!) as! FileManagerCell
            fileData.copyFile(currentCell.fileName)
            reloadData()
        }
    }



    func actionLevelUp()
    {
        fileData.leaveDirectory()
        reloadData()
    }



    func actionCreateDirectory(_ name : String)
    {
        fileData.createDirectory(name)
        reloadData()
    }



    // Table handling functions
    override func numberOfSections(in tableView: UITableView) -> Int
    {
        return 1
    }



    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int
    {
        return fileData.currentDirList.count + fileData.currentFileList.count
    }



    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell
    {
        let cell = self.tableView.dequeueReusableCell(withIdentifier: "fileEntry", for: indexPath) as! FileManagerCell
        let row  = indexPath.row

        if row < fileData.currentDirList.count {
            cell.fileName = fileData.currentDirList[row]
            cell.fileLabel.text = cell.fileName + "/"
            cell.isDirectory  = true
        } else {
            let inx = row - fileData.currentDirList.count
            cell.fileName = fileData.currentFileList[inx]
            cell.fileLabel.text = cell.fileName
            cell.isDirectory  = false
        }
        return cell
    }



    // Select a row (file) and show actions
    override  func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath)
    {
        selectedRow = indexPath
        performSegue(withIdentifier: "showActions", sender: self)
    }



    // Support function
    func reloadData()
    {
        title = (fileData.selectStorage(false) ? "Local - " : "iCloud - ") + fileData.getDirectory()
        tableView.reloadData()
    }
}



// Space holder for extra information needed to do the right thing for each action
class FileManagerCell: UITableViewCell {

    @IBOutlet weak var fileLabel: UILabel!
    var isDirectory : Bool = false
    var fileName    : String = ""
}



// Protocol for action popover callback
protocol FileActionsControlDelegate
{
    func actionOpen()
    func actionDelete()
    func actionRename(_ name : String)
    func actionUploadDownload()
    func actionLevelUp()
    func actionCreateDirectory(_ name : String)
}



// Action popover dialog
class FileManagerActions : UITableViewController

{
    // Pointer to callback class
    var delegate : FileActionsControlDelegate?
    var inSubDirectory : Bool = false
    var inFileSelect   : Bool = false
    var useCloud       : Bool = false

    // Calling class might enable/disable each button
    @IBOutlet weak var buttonUploadDownload: UIButton!
    @IBOutlet weak var buttonDelete: UIButton!
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonRename: UIButton!
    @IBOutlet weak var buttonLevelUp: UIButton!


    // Actions
    @IBAction func doOpen(_ sender: UIButton)
    {
        delegate?.actionOpen()
        dismiss(animated: false)
    }



    @IBAction func doDelete(_ sender: UIButton)
    {
        delegate?.actionDelete()
        dismiss(animated: false)
    }



    @IBAction func doUploadDownload(_ sender: UIButton)
    {
        delegate?.actionUploadDownload()
        dismiss(animated: false)
    }



    @IBAction func doLevelUp(_ sender: UIButton)
    {
        delegate?.actionLevelUp()
        dismiss(animated: false)
    }



    override func viewDidLoad()
    {
        super.viewDidLoad()
        buttonLevelUp.isEnabled = inSubDirectory
        buttonDelete.isEnabled = inFileSelect
        buttonOpen.isEnabled = inFileSelect
        buttonRename.isEnabled = inFileSelect
        buttonUploadDownload.isEnabled = (inFileSelect && useCloud)
    }



    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        let vc = segue.destination as! setNameAction
        vc.delegateFile = self.delegate
        vc.protocolActionToPerform = (segue.identifier == "showRename") ? 0 : 1
    }
}



// Action popover dialog
class setNameAction : UIViewController

{
    // Pointer to callback class
    var delegateFile : FileActionsControlDelegate?
    var delegateDoc  : DocumentActionsControlDelegate?
    var protocolActionToPerform : Int = -1


    // Calling class might enable/disable each button
    @IBOutlet weak var editText: UITextField!



    @IBAction func doOK(_ sender: UIButton)
    {
        switch protocolActionToPerform
        {
            case 0: // renameDir
                delegateFile?.actionRename(editText.text!)
            case 1: // createDir
                delegateFile?.actionCreateDirectory(editText.text!)
            case 2: // New
                delegateDoc?.actionNew(editText.text!)
            case 3: // SaveAs
                delegateDoc?.actionSaveAs(editText.text!)
            default:
                break
        }
        dismiss(animated: false)
    }



    override func viewDidLoad()
    {
        super.viewDidLoad()
    }
}

