/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "slideShowPreviewTable_vc.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"

@interface slideShowPreviewTable_vc ()

@property (nonatomic, weak) UIButton * startButton;
@property (nonatomic, strong) NSArray * optionsArray;
@property (nonatomic, strong) CommunicationManager * comManager;
@property (nonatomic, strong) id slideShowStartObserver;
@property BOOL slidesRunning;

@end

@implementation slideShowPreviewTable_vc

@synthesize startButton = _startButton;
@synthesize optionsTable = _optionsTable;
@synthesize optionsArray = _optionsArray;

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    self.optionsArray = [NSArray arrayWithObjects:OPTION_TIMER, OPTION_POINTER, nil];
    self.comManager = [CommunicationManager sharedComManager];
    self.comManager.delegate = self;
    self.slidesRunning = NO;
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                    object:nil
                                                                                     queue:mainQueue
                                                                                usingBlock:^(NSNotification *note) {
                                                                                    self.slidesRunning = YES;
                                                                                }];
    }

- (void) viewDidAppear:(BOOL)animated
{
    if (self.slidesRunning && [self.comManager.interpreter.slideShow size] > 0){
        self.slidesRunning = NO;
        [self performSegueWithIdentifier:@"slideShowSegue" sender:self];
    }
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                    object:nil
                                                                                     queue:mainQueue
                                                                                usingBlock:^(NSNotification *note) {
//                                                                                    if (NSClassFromString(@"NSLayoutConstraint"))
//                                                                                        // support autolayout, this tablevc has a parent_vc which has this segue
//                                                                                        [self.parentViewController performSegueWithIdentifier:@"slideShowSegue" sender:self];
//                                                                                    else
                                                                                      [self performSegueWithIdentifier:@"slideShowSegue" sender:self];
                                                                                }];
    [super viewDidAppear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
    self.slideShowStartObserver = nil;
    self.slidesRunning = NO;
    [super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.optionsArray count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"optionCell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
        UISwitch *toggleSwitch = [[UISwitch alloc] init];
        cell.accessoryView = [[UIView alloc] initWithFrame:toggleSwitch.frame];
        if (indexPath.row == 0) {
            [toggleSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:KEY_TIMER]];
        } else {
            [toggleSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:KEY_POINTER]];
        }
        [cell.accessoryView addSubview:toggleSwitch];
    }
    cell.textLabel.text = [self.optionsArray objectAtIndex:indexPath.row];
    
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

-(IBAction)startPresentationAction:(id)sender {
    for (UITableViewCell *cell in self.tableView.visibleCells) {
        UISwitch * toggle = [[[cell accessoryView] subviews] objectAtIndex:0];

        if ([cell.textLabel.text isEqualToString:OPTION_TIMER])
            [[NSUserDefaults standardUserDefaults] setBool:[toggle isOn] forKey:KEY_TIMER];
        else if ([cell.textLabel.text isEqualToString:OPTION_POINTER])
            [[NSUserDefaults standardUserDefaults] setBool:[toggle isOn] forKey:KEY_POINTER];
    }
    [[self.comManager transmitter] startPresentation];
}

- (UIButton *)startButton{
    if (_startButton == nil) {
        _startButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        _startButton.frame = CGRectMake(10.0, 10.0, 300.0, 50.0);
        [_startButton setTitle:@"Start Presentation" forState:UIControlStateNormal];
        [_startButton addTarget:self action:@selector(startPresentationAction:) forControlEvents:UIControlEventTouchDown];
        
        _startButton.tag = 1;
    }
    return _startButton;
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
    
    UIView* customView = [[UIView alloc] initWithFrame:CGRectMake(0.0, 0.0, 320.0, 100.0)];
    
    [customView addSubview:self.startButton];
    
    return customView;
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    return 50.0;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell * cell = [tableView cellForRowAtIndexPath:indexPath];
    UISwitch * toggle = [[[cell accessoryView] subviews] objectAtIndex:0];
    [toggle setOn:![toggle isOn] animated:YES];
    [cell setSelected:NO animated:YES];
}

- (void)viewDidUnload {
    [self setOptionsTable:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
    self.slideShowStartObserver = nil;
    [super viewDidUnload];
}
@end
