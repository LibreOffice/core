// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <dispatch/dispatch.h>
#import "Client.h"
#import "Server.h"
#import "CommandInterpreter.h"
#import "CommunicationManager.h"

#define CHARSET @"UTF-8"

@interface Client() <NSStreamDelegate>

@property (nonatomic, strong) NSInputStream* inputStream;
@property (nonatomic, strong) NSOutputStream* outputStream;

@property uint mPort;
@property (nonatomic, weak) CommandInterpreter* receiver;
@property (nonatomic, weak) CommunicationManager* comManager;

@property (nonatomic, strong) NSTimer* connectionTimeoutTimer;

@end

@implementation Client

@synthesize inputStream = _mInputStream;
@synthesize outputStream = _mOutputStream;
@synthesize pin = _mPin;
@synthesize name = _mName;
@synthesize server = _mServer;
@synthesize comManager = _mComManager;
@synthesize connected = _mReady;
@synthesize receiver = _receiver;
@synthesize connectionTimeoutTimer = _connectionTimeoutTimer;


dispatch_queue_t backgroundQueue;

- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(CommandInterpreter*)receiver
{
    self = [self init];
    if (self)
    {
        self.connected = NO;
        self.name = [[UIDevice currentDevice] name];
        self.pin = [NSNumber numberWithInteger:[self getPin]];
        self.server = server;
        self.comManager = manager;
        self.receiver = receiver;
        self.mPort = 1599;
    }
    return self;
}

- (void)startConnectionTimeoutTimer
{
    [self stopConnectionTimeoutTimer]; // Or make sure any existing timer is stopped before this method is called

    NSTimeInterval interval = 3.0; // Measured in seconds, is a double

    self.connectionTimeoutTimer = [NSTimer scheduledTimerWithTimeInterval:interval
                                                                   target:self
                                                                 selector:@selector(handleConnectionTimeout)
                                                                 userInfo:nil
                                                                  repeats:NO];
}

- (void)handleConnectionTimeout
{
    NSLog(@"handleConnectionTimeout");
    [self disconnect];
    [[NSNotificationCenter defaultCenter]postNotificationName:@"connection.status.disconnected" object:nil];
}

- (void)dealloc
{
    dispatch_release(backgroundQueue);
    [self stopConnectionTimeoutTimer];
}

- (void)stopConnectionTimeoutTimer
{
    if (self.connectionTimeoutTimer)
    {
        [self.connectionTimeoutTimer invalidate];
        self.connectionTimeoutTimer = nil;
    }
}

- (NSInteger) getPin
{
    // Look up if there is already a pin code for this client.
    NSUserDefaults * userDefaluts = [NSUserDefaults standardUserDefaults];

    if(!userDefaluts)
        NSLog(@"userDefaults nil");
    NSInteger newPin = [userDefaluts integerForKey:self.name];

    // If not, generate one.
    if (!newPin) {
        newPin = arc4random() % 9999;
        [userDefaluts setInteger:newPin forKey:self.name];
    }

    return newPin;
}

- (void)streamOpenWithIp:(NSString *)ip withPortNumber:(uint)portNumber
{
    NSLog(@"Connecting to %@:%u", ip, portNumber);
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)ip, portNumber, &readStream, &writeStream);

    if(readStream && writeStream)
    {
        CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);

        //Setup mInputStream
        self.inputStream = (__bridge NSInputStream *)readStream;
        [self.inputStream setDelegate:self];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        });
        [self.inputStream open];

        //Setup outputstream
        self.outputStream = (__bridge NSOutputStream *)writeStream;
        [self.outputStream setDelegate:self];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        });
        [self.outputStream open];

        NSArray *temp = [[NSArray alloc]initWithObjects:@"LO_SERVER_CLIENT_PAIR\n", self.name, @"\n", self.pin, @"\n\n", nil];
        NSString *command = [temp componentsJoinedByString:@""];

        [self sendCommand:command];
    }
}

- (void) sendCommand:(NSString *)aCommand
{
    NSLog(@"Sending command %@", aCommand);
    // UTF-8 as speficied in specification
    NSData * data = [aCommand dataUsingEncoding:NSUTF8StringEncoding];

    [self.outputStream write:(uint8_t *)[data bytes] maxLength:[data length]];
}

int count = 0;

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode {
    switch(eventCode) {
        case NSStreamEventOpenCompleted:{
            [self stopConnectionTimeoutTimer];
            [[NSNotificationCenter defaultCenter]postNotificationName:@"connection.status.connected" object:nil];
        }
            break;
        case NSStreamEventErrorOccurred:{
            [self stopConnectionTimeoutTimer];
            [self disconnect];
            NSLog(@"Connection error occured");
            [[NSNotificationCenter defaultCenter]postNotificationName:@"connection.status.disconnected" object:nil];
        }
            break;
        case NSStreamEventHasBytesAvailable:
        {
            NSMutableData* data;
            //            NSLog(@"NSStreamEventHasBytesAvailable");
            if(!data) {
                data = [[NSMutableData alloc] init];
            }
            uint8_t buf[1024];
            unsigned int len = 0;
            NSString *str;
            while (true) {
                len = [(NSInputStream *)stream read:buf maxLength:1024];
                [data appendBytes:(const void *)buf length:len];
                if (len < 1024) {
                    // Potentially the end of a command
                    str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
                    if ([str hasSuffix:@"\n\n"]) {
                        // Finished current command
                        break;
                    }
                }
            }
//            NSLog(@"Command:%@", str);
            NSArray *commands = [str componentsSeparatedByString:@"\n"];
            [self.receiver parse:commands];
            data = nil;
            str = nil;
        } break;
        default:
        {

        }

    }
}

- (void) disconnect
{
    if(self.inputStream == nil && self.outputStream == nil)
        return;
    [self.inputStream close];
    [self.outputStream close];
    [self.inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [self.outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    self.inputStream = nil;
    self.outputStream = nil;
    self.connected = NO;
}

- (void) connect
{
    [self startConnectionTimeoutTimer];
    backgroundQueue = dispatch_queue_create("com.libreoffice.iosremote", NULL);
    dispatch_async(backgroundQueue, ^(void) {
        [self streamOpenWithIp:self.server.serverAddress withPortNumber:self.mPort];
    });
}



@end
