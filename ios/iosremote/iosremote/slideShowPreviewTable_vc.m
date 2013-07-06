//
//  slideShowPreviewTable_vc.m
//  iosremote
//
//  Created by Liu Siqi on 7/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowPreviewTable_vc.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"

@interface slideShowPreviewTable_vc ()

@property (nonatomic, weak) UIButton * startButton;
@property (nonatomic, strong) NSArray * optionsArray;
@property (nonatomic, strong) CommunicationManager * comManager;
@property (nonatomic, strong) id slideShowStartObserver;

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
    self.optionsArray = [NSArray arrayWithObjects:@"Lecturer's Notes", @"Timer", @"Pointer", nil];
    self.comManager = [CommunicationManager sharedComManager];
    self.comManager.delegate = self;
    }

- (void) viewDidAppear:(BOOL)animated
{
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                    object:nil
                                                                                     queue:mainQueue
                                                                                usingBlock:^(NSNotification *note) {
                                                                                    [self.parentViewController performSegueWithIdentifier:@"slideShowSegue" sender:self];
                                                                                }];
    [super viewDidAppear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
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
        [toggleSwitch setOn:YES];
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
    [[self.comManager transmitter] startPresentation];
}

- (UIButton *)startButton{
    if (_startButton == nil) {
        _startButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        _startButton.frame = CGRectMake(10.0, 10.0, 300.0, 50.0);
        [_startButton setTitle:@"Start Presentation" forState:UIControlStateNormal];
        _startButton.backgroundColor = [UIColor clearColor];
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
}

- (void)viewDidUnload {
    [self setOptionsTable:nil];
    [super viewDidUnload];
}
@end
